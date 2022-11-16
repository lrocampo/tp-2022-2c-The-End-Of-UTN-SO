#include <cpu_utils.h>

t_log *cpu_logger;
t_cpu_config* cpu_config;
t_memoria_config* memoria_config;
int server_fd_dispatch;
int cliente_fd_dispatch;
int conexion_memoria;
bool interrupcion;

pthread_t th_kernel_dispatch;
pthread_t th_kernel_interrupt;

pthread_mutex_t interrupcion_mutex;
t_pagina_config* pagina_config;

/* Configuracion y Limpieza */

void * configurar_cpu(t_config* config){
	t_cpu_config* cpu_config;
	cpu_config = malloc(sizeof(t_cpu_config));
	cpu_config->ip_cpu = strdup(config_get_string_value(config, "IP_CPU"));
	cpu_config->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
	cpu_config->ip_memoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	cpu_config->puerto_memoria = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	cpu_config->puerto_escucha_dispatch = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));
	cpu_config->puerto_escucha_interrupt = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"));
	cpu_config->retardo_intruccion = config_get_int_value(config, "RETARDO_INSTRUCCION");
	return cpu_config;
}

void* configurar_tlb(t_config* config){
	t_tlb_config* tlb_config;
	tlb_config = malloc(sizeof(t_tlb_config));
	tlb_config-> entradas_tlb = strdup(config_get_string_value(config, "ENTRADAS_TLB"));
    tlb_config-> reemplazo_tlb = strdup(config_get_string_value(config, "REEMPLAZO_TLB"));
	tlb_config-> retardo_instruccion = strdup(config_get_string_value(config, "RETARDO_INSTRUCCION"));
	tlb_config-> ip_memoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	tlb_config-> puerto_memoria = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	tlb_config-> puerto_escucha_dispatch = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));
	tlb_config-> puerto_escucha_interrupt = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"));
	return tlb_config;
}

//Hacer malloc tanto de pagina como de marco

void iniciar_conexion_con_memoria() {
	conexion_memoria = crear_conexion(cpu_config->ip_memoria, cpu_config->puerto_memoria);
	if(conexion_memoria != -1){
		log_debug(cpu_logger, "Conexion creada correctamente con MEMORIAs");
	}

	cod_mensaje cod_msj = recibir_operacion(conexion_memoria);

	if(cod_msj == HANDSHAKE) {
		pagina_config = recibir_configuracion_memoria(conexion_memoria);
		log_debug(cpu_logger, "Handshake con memoria realizado.\nTamanio Pagina: %d Kbytes.\nCantidad de entradas por tabla: %d.", pagina_config->tamanio_pagina, pagina_config->cantidad_entradas);
	}
	else {
		error_show("Error en handshake.");
	}
}

void cpu_config_destroy(){
	free(cpu_config->ip_cpu);
	free(cpu_config->ip_kernel);
	free(cpu_config->ip_memoria);
	free(cpu_config->puerto_escucha_dispatch);
	free(cpu_config->puerto_escucha_interrupt);
	free(cpu_config->puerto_memoria);
	free(cpu_config);
}

void esperar_conexiones() {
    pthread_join(th_kernel_dispatch, NULL);
}

void terminar_modulo(){
	liberar_conexion(conexion_memoria);
	log_destroy(cpu_logger);
	cpu_config_destroy();
}

/* Ciclo de instruccion */
void ciclo_de_instruccion_init() {
	pthread_create(&th_kernel_dispatch, NULL, &atender_kernel_dispatch, NULL);

	pthread_create(&th_kernel_interrupt, NULL, &atender_kernel_interrupt, NULL);
	pthread_detach(th_kernel_interrupt);
}

void iniciar_ciclo_de_instruccion(t_pcb* pcb_to_exec) {
	log_debug(cpu_logger, "Iniciando ciclo de instruccion");
	instruccion* instruccion;
	cod_operacion operacion_a_ejecutar;

	while(pcb_to_exec->program_counter < list_size(pcb_to_exec->instrucciones)) {

		// fetch()
		instruccion = fetch(pcb_to_exec);

		// decode()
		operacion_a_ejecutar = decode(pcb_to_exec, instruccion);

		// execute()
		ejecutar_instruccion(pcb_to_exec, operacion_a_ejecutar, instruccion);

		pcb_to_exec->program_counter++;

		// Chequear si este orden esta bien (1. block por IO 2. interrupcion)
		if(operacion_a_ejecutar == IO) break;

		// Check interrupt
		if(interrupcion) {
			log_debug(cpu_logger, "Se recibio señal de interrupcion");
			pcb_to_exec->interrupcion = true;
			pthread_mutex_lock(&interrupcion_mutex);
			interrupcion = false;
			pthread_mutex_unlock(&interrupcion_mutex);
			break;
		}

	}
}

instruccion* fetch(t_pcb* pcb_to_exec) {
	t_list* instrucciones = pcb_to_exec->instrucciones;
	instruccion* instruccion_a_ejecutar = list_get(instrucciones, (int) pcb_to_exec->program_counter);

	return instruccion_a_ejecutar;
}

cod_operacion decode(t_pcb* pcb_to_exec, instruccion* instruccion_a_decodificar) {
	cod_operacion operacion = instruccion_a_decodificar->operacion;
	char* registro;
	int dir_logica;
	t_marco* marco;
	t_pagina* pagina;
	int nro_pagina;
	int desplazamiento_segmento;
	t_segmento* segmento;
	int resultado_pedir_marco;
	int offset;
	int cantidad_entradas; //= memoria_config->entradas_por_tabla;
	int tamanio_pagina; //= memoria_config->tamanio_pagina;
	int tam_max_segmento = cantidad_entradas * tamanio_pagina;
	if(operacion == MOV_IN || operacion == MOV_OUT){
		
		if(operacion == MOV_IN) {
		registro = instruccion_a_decodificar->parametro1
		dir_logica = (int) strtol(instruccion_a_decodificar->parametro2, NULL, 10);
	    }
	    if(operacion == MOV_OUT){
		registro = instruccion_a_decodificar->parametro2
		dir_logica = (int) strtol(instruccion_a_decodificar->parametro1, NULL, 10);
	    }
		
		desplazamiento_segmento = obtener_desplazamiento_segmento(dir_logica, tam_max_segmento);
		//Si hay segmentation fault, envio mensaje a kernel con el pcb sin actualizar el pc
		if(desplazamiento_segmento > tam_max_segmento){//(*)tam_max_segmento o tam_segmento ??
			 enviar_mensaje("segmentation fault", conexion_kernel);
			 puts("Enviano pcb sin pc actualizado");

		}

		nro_pagina = obtener_nro_pagina(dir_logica, tam_max_segmento, tamanio_pagina);
	    resultado_pedir_marco = buscar_en_tlb(pcb_to_exec->pid, nro_pagina);
		
	    // si la tlb no encontró la pagina, envia mensaje a memoria solicitando el marco
	    if (resultado == -1) {
			enviar_mensaje("no se encontró la pagina en la tlb", conexion_memoria);
			puts("Enviando nro de pagina y pid");
			//si la memoria devuelve page fault, envio mensaje a kernel con el pcb sin actualizar el pc
			if(mensaje == PAGE_FAULT){
				enviar_mensaje("la memoria devolvió page fault", conexion_kernel);
			    puts("Enviano pcb sin pc actualizado");
			}
			
	    } 
	}
	

	return operacion;
}

void ejecutar_instruccion(t_pcb* pcb, cod_operacion operacion_a_ejecutar, instruccion* instruccion){
	char* operacion_string = strdup(operacion_to_string(operacion_a_ejecutar));
	if(operacion_a_ejecutar != EXIT) {
		log_info(cpu_logger, "PID: %d - Ejecutando %s - %s %s", (int)pcb->pid, operacion_string, instruccion->parametro1, instruccion->parametro2);
	}
	else {
		log_info(cpu_logger, "PID: %d - Ejecutando %s", (int)pcb->pid, operacion_string);
	}

	switch(operacion_a_ejecutar) {
		case SET:
			ejecutar_set(pcb, instruccion->parametro1, instruccion->parametro2);
			valor_retardo_instruccion(cpu_config->retardo_intruccion);
			break;
		case ADD:
			ejecutar_add(pcb, instruccion->parametro1, instruccion->parametro2);
			valor_retardo_instruccion(cpu_config->retardo_intruccion);
			break;
		case MOV_IN:
			ejecutar_mov_in(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case MOV_OUT:
			ejecutar_mov_out(pcb, instruccion->parametro1, instruccion->parametro2);
			break;
		case IO:
			break;
		case EXIT:
			break;
		default:
			error_show("Error, instruccion desconocida.");												
	}
	free(operacion_string);
}

void ejecutar_set(t_pcb* pcb, char* parametro1, char* parametro2) {
	set_valor_registro(pcb, parametro1, parametro2);
}

void ejecutar_add(t_pcb* pcb, char* parametro1, char* parametro2) {	
	uint32_t valorRegistroDestino = obtener_valor_del_registro(pcb, parametro1);
	uint32_t valorRegistroOrigen = obtener_valor_del_registro(pcb, parametro2); 

	valorRegistroDestino += valorRegistroOrigen;

	// Transformo el entero a string para poder reutilizar ejecutar_set, que espera que el parametro2 sea un string.
	char *resultado_a_string = string_itoa(valorRegistroDestino);

	ejecutar_set(pcb, parametro1, resultado_a_string);
	free(resultado_a_string);
}

void ejecutar_mov_in(t_pcb* pcb, char* parametro1, char* parametro2) {
	log_debug(cpu_logger,"MOV_IN: no hago nada, todavia....");
}

void ejecutar_mov_out(t_pcb* pcb, char* parametro1, char* parametro2) {
	log_debug(cpu_logger,"MOV_OUT: no hago nada, todavia....");
}

/* Retardo de instruccion */
void valor_retardo_instruccion(uint32_t tiempo){
	ejecutar_espera(tiempo);
}


/* Conexiones con Kernel */
void* atender_kernel_dispatch(void* arg) {
	server_fd_dispatch = iniciar_servidor(cpu_config->ip_cpu, cpu_config->puerto_escucha_dispatch);
	
	if(server_fd_dispatch == -1){
		log_error(cpu_logger, "Hubo un error inicializando el servidor.");
        pthread_exit(NULL);
	}

	cliente_fd_dispatch = esperar_cliente(server_fd_dispatch);

	if(cliente_fd_dispatch == -1){
		log_error(cpu_logger, "Hubo un error conectandose con el cliente.");
        pthread_exit(NULL);
	}

	log_debug(cpu_logger,"Se conecto un cliente a DISPATCH");

	while(1) {
		t_pcb* pcb_to_exec;
		int cod_op = recibir_operacion(cliente_fd_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cpu_logger, cliente_fd_dispatch);
			break;
		case PCB:
			pcb_to_exec = recibir_pcb(cliente_fd_dispatch);
			log_debug(cpu_logger, "Recibi pcb con pid: %d",pcb_to_exec->pid);
			// Fetch -> Decode -> Execute -> Check Interrupt
			iniciar_ciclo_de_instruccion(pcb_to_exec);
			enviar_pcb(pcb_to_exec, cliente_fd_dispatch);
			pcb_destroy(pcb_to_exec);
			break;
		case -1:
			log_debug(cpu_logger, "El cliente se desconecto de DISPATCH");
			pthread_exit(NULL);
		default:
			break;
		}
	}
}

void* atender_kernel_interrupt(void* arg) {
	int server_fd_interrupt = iniciar_servidor(cpu_config->ip_cpu, cpu_config->puerto_escucha_interrupt);
	int cliente_fd_interrupt = esperar_cliente(server_fd_interrupt);
	log_debug(cpu_logger,"Se conecto un cliente a INTERRUPT");

	while(1){
		
		cod_mensaje cod_op = recibir_operacion(cliente_fd_interrupt);

		if(cod_op == INTERRUPCION) {
			pthread_mutex_lock(&interrupcion_mutex);
			interrupcion = true;
			pthread_mutex_unlock(&interrupcion_mutex);
		}
		else {
			error_show("Error, se recibio algo que no es una interrupcion: %d\n", cod_op);
			pthread_exit(NULL);
		}
		
		if(cliente_fd_interrupt == -1){
			error_show("Error conectando con el kernel");
			log_debug(cpu_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
		}
	}
}







							



