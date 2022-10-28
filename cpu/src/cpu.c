/*
 * cpu.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */
#include <cpu.h>

int main(void){
	//t_cpu_config* cpu_config;

	/* LOGGER DE ENTREGA */
	/* cpu_logger = iniciar_logger(RUTA_LOGGER_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_INFO); */

	/* LOGGER DE DEBUG */
	cpu_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(cpu_logger,"Arrancando cpu");

	cpu_config = cargar_configuracion(RUTA_CPU_CONFIG, CPU);
	log_debug(cpu_logger,"Configuracion cargada correctamente");

	server_fd_dispatch = iniciar_servidor(cpu_config->ip_cpu, cpu_config->puerto_escucha_dispatch);

	iniciar_conexion_con_memoria();

	if(server_fd_dispatch == -1){
		return EXIT_FAILURE;
	}

	cliente_fd_dispatch = esperar_cliente(server_fd_dispatch);
	log_debug(cpu_logger,"Se conecto un cliente a DISPATCH");

	if(cliente_fd_dispatch == -1){
		return EXIT_FAILURE;
	}

	// en otro hilo:
	pthread_t thread_kernel_interrupt;
	pthread_create(&thread_kernel_interrupt, NULL, &atender_kernel_interrupt, NULL);

	while(1){
			t_pcb* pcb_to_exec;
			int cod_op = recibir_operacion(cliente_fd_dispatch);
			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cpu_logger, cliente_fd_dispatch);
				break;
			case PCB:
				pcb_to_exec = recibir_pcb(cliente_fd_dispatch);
				log_debug(cpu_logger, "Recibi pcb con pid: %d",pcb_to_exec->pid);
				char* pcb_string = pcb_to_string(pcb_to_exec);
				log_debug(cpu_logger, "PCB RECIBIDA:\n %s", pcb_string);

				// Fetch -> Decode -> Execute -> Check Interrupt
				iniciar_ciclo_de_instruccion(pcb_to_exec);

				enviar_pcb(pcb_to_exec, cliente_fd_dispatch);
				free(pcb_string);
				pcb_destroy(pcb_to_exec);
				break;
			case -1:
				log_debug(cpu_logger, "El cliente se desconecto de DISPATCH");
				return EXIT_FAILURE;
			default:
				break;
			}
		}

	/*while(1){
				int cod_op = recibir_operacion(cliente_fd_interrupt);
				switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd_interrupt);
					break;
				case -1:
					break;
				default:
					break;
				}
				break;
			}
*/
	log_debug(cpu_logger,"termino cpu\n");
	log_destroy(cpu_logger);
	return EXIT_SUCCESS;
}

void iniciar_ciclo_de_instruccion(t_pcb* pcb_to_exec) {
	log_debug(cpu_logger, "Iniciando ciclo de instruccion");
	instruccion* instruccion;
	cod_operacion operacion_a_ejecutar;
	while(pcb_to_exec->program_counter < list_size(pcb_to_exec->instrucciones)) {

		// fetch()
		instruccion = fetch(pcb_to_exec);

		// decode()
		operacion_a_ejecutar = decode(instruccion);
		

		// execute()
		ejecutar_instruccion(pcb_to_exec, operacion_a_ejecutar, instruccion);

		// Check interrupt
		if(interrupcion) {
			log_debug(cpu_logger, "Se recibio señal de interrupcion");
			pcb_to_exec->interrupcion = true;
			/* La linea de abajo podria ser responsabilidad del kernel, y no de la cpu */
			pcb_to_exec->estado = READY;
			pthread_mutex_lock(&interrupcion_mutex);
			interrupcion = false;
			pthread_mutex_unlock(&interrupcion_mutex);
			break;
		}
	}

	if(pcb_to_exec->program_counter == list_size(pcb_to_exec->instrucciones)) {
		pcb_to_exec->estado = FINISH_EXIT;
	}
}

void* atender_kernel_interrupt(void* arg) {
	int server_fd_interrupt = iniciar_servidor(cpu_config->ip_cpu, cpu_config->puerto_escucha_interrupt);
	int cliente_fd_interrupt = esperar_cliente(server_fd_interrupt);

	log_debug(cpu_logger,"Se conecto un cliente a INTERRUPT");

	int cod_op = recibir_operacion(cliente_fd_interrupt);

	if(cod_op == INTERRUPCION) {
		pthread_mutex_lock(&interrupcion_mutex);
		interrupcion = true;
		pthread_mutex_unlock(&interrupcion_mutex);
	}

	else {
		error_show("Error.");
	}

	if(cliente_fd_interrupt == -1){
		error_show("Error conectando con el kernel");
	}
}


instruccion* fetch(t_pcb* pcb_to_exec) {
	t_list* instrucciones = pcb_to_exec->instrucciones;
	instruccion* instruccion_a_ejecutar = list_get(instrucciones, (int) pcb_to_exec->program_counter);

	return instruccion_a_ejecutar;
}

cod_operacion decode(instruccion* instruccion_a_decodificar) {
	cod_operacion operacion = instruccion_a_decodificar->operacion;
	if(operacion == MOV_IN || operacion == MOV_OUT) {
		enviar_mensaje("pido memoria", conexion_memoria);
		recibir_mensaje(cpu_logger, conexion_memoria);
	}

	return operacion;
}

void ejecutar_instruccion(t_pcb* pcb, cod_operacion operacion_a_ejecutar, instruccion* instruccion){
	log_debug(cpu_logger, "Ejecutando instruccion...");

	switch(operacion_a_ejecutar) {
		case SET:
			ejecutar_set(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case ADD:
			ejecutar_add(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case MOV_IN:
			ejecutar_mov_in(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case MOV_OUT:
			ejecutar_mov_out(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case IO:
			ejecutar_io(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case EXIT:
			ejecutar_exit(pcb);
			break;												
	}

	sleep(5);
	printf("%d\n", list_size(pcb->instrucciones));
	pcb->program_counter = list_size(pcb->instrucciones); 
}

void ejecutar_set(t_pcb* pcb, char* parametro1, char* parametro2) {
	if(string_equals_ignore_case(parametro1, "ax")) {
		pcb->registros.ax = (uint32_t) atoi(parametro2);
	}
	else if(string_equals_ignore_case(parametro1, "bx")) {
		pcb->registros.bx = (uint32_t) atoi(parametro2);
	}
	else if(string_equals_ignore_case(parametro1, "cx")) {
		pcb->registros.cx = (uint32_t) atoi(parametro2);
	}
	else if(string_equals_ignore_case(parametro1, "dx")) {
		pcb->registros.dx = (uint32_t) atoi(parametro2);
	}	
}

void ejecutar_add(t_pcb* pcb, char* parametro1, char* parametro2) {
	uint32_t valorRegistroDestino = obtener_valor_del_registro(pcb, parametro1);
	uint32_t valorRegistroOrigen = obtener_valor_del_registro(pcb, parametro2);

	valorRegistroDestino += valorRegistroOrigen;

	// Transformo el entero a string para poder reutilizar ejecutar_set, que espera que el parametro2 sea un string.
	char *resultado_a_string = string_itoa(valorRegistroDestino);

	ejecutar_set(pcb, parametro1, resultado_a_string);

	// pcb->registros->ax = pcb->registros->ax + pcb->registros->bx
}

void ejecutar_mov_in(t_pcb* pcb, char* parametro1, char* parametro2) {
	puts("Ejecutando mov_in (no estoy haciendo nada xd)");
}

void ejecutar_mov_out(t_pcb* pcb, char* parametro1, char* parametro2) {
	puts("Ejecutando mov_out (no estoy haciendo nada xd)");
}


void ejecutar_io(t_pcb* pcb, char* parametro1, char* parametro2) {
	pcb->contexto_de_io.dispositivo = strdup(parametro1);
}

uint32_t obtener_valor_del_registro(t_pcb* pcb, char* parametro1) {
	uint32_t valor_de_registro;
	if(string_equals_ignore_case(parametro1, "ax")) {
		valor_de_registro = pcb->registros.ax;
	}
	if(string_equals_ignore_case(parametro1, "bx")) {
		valor_de_registro = pcb->registros.bx;
	}
	if(string_equals_ignore_case(parametro1, "cx")) {
		valor_de_registro = pcb->registros.cx;
	}
	if(string_equals_ignore_case(parametro1, "dx")) {
		valor_de_registro = pcb->registros.dx;
	}

	return valor_de_registro;
}

void iniciar_conexion_con_memoria() {
	conexion_memoria = crear_conexion(cpu_config->ip_memoria, cpu_config->puerto_memoria);
	if(conexion_memoria != -1){
		log_debug(cpu_logger, "Conexion creada correctamente con MEMORIAs");
	}
}