#include <cpu_utils.h>

t_log *cpu_logger;
t_cpu_config* cpu_config;
//t_memoria_config* memoria_config;
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

void set_dir_fisica_a_instruccion(instruccion* instruccion, int dir_fisica) {
	cod_operacion operacion = instruccion->operacion;
	if(operacion == MOV_IN) {
		free(instruccion->parametro2);
		instruccion->parametro2 = string_itoa(dir_fisica);
	}
	if(operacion == MOV_OUT) {
		free(instruccion->parametro1);
		instruccion->parametro1 = string_itoa(dir_fisica);
	}
}

int obtener_dir_logica(instruccion* instruccion) {
	cod_operacion operacion = instruccion->operacion;

	if(operacion == MOV_IN) {
		return atoi(instruccion->parametro2);
	}
	else {
		return atoi(instruccion->parametro1);
	}
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

		if(operacion_a_ejecutar == ERROR_MEMORIA) break;

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
	int dir_logica;
	t_pagina* pagina = malloc(sizeof(t_pagina));
	int nro_pagina;
	int desplazamiento_segmento;
	int resultado_pedir_marco;
	int tam_max_segmento = pagina_config->cantidad_entradas * pagina_config->tamanio_pagina;
	int nro_marco;
	int desplazamiento_pagina;
	int direccion_fisica;

	if(operacion == MOV_IN || operacion == MOV_OUT){
		dir_logica = obtener_dir_logica(instruccion_a_decodificar);
		
		t_segmento* segmento = list_get(pcb_to_exec->tabla_de_segmentos, obtener_nro_segmento(dir_logica, tam_max_segmento));


		desplazamiento_segmento = obtener_desplazamiento_segmento(dir_logica, tam_max_segmento);
		//Si hay segmentation fault, envio mensaje a kernel con el pcb sin actualizar el pc
		if(desplazamiento_segmento > segmento->tamanio_segmento){
			pcb_to_exec->segmentation_fault = true;
			free(pagina);
			return ERROR_MEMORIA;
		}

		nro_pagina = obtener_nro_pagina(dir_logica, tam_max_segmento, pagina_config->tamanio_pagina);
	    resultado_pedir_marco = buscar_en_tlb(pcb_to_exec->pid, nro_pagina);
		
	    // si la tlb no encontró la pagina, envia mensaje a memoria solicitando el marco
	    if (resultado_pedir_marco == -1) {
			// TLB MISS
			pagina->numero_pagina = nro_pagina;
			pagina->indice_tabla_de_pagina = segmento->indice_tabla_paginas;
			log_debug(cpu_logger, "enviando pagina: %d", pagina->numero_pagina);
			enviar_pagina(pagina, conexion_memoria);

			cod_mensaje cod_msj = recibir_operacion(conexion_memoria);
			if(cod_msj == OKI_MARCO) {
				log_debug(cpu_logger, "Memoria encontro el numero de marco");
				nro_marco = recibir_valor(conexion_memoria);
			}
			else if(cod_msj == PAGE_NOT_FOUND_404) {
				log_debug(cpu_logger, "Memoria no encontro el numero de marco. PAGE FAULT. Pagina: %d", pagina->numero_pagina);
				pcb_to_exec->page_fault = true;
				pcb_to_exec->pagina_fault->indice_tabla_de_pagina = pagina->indice_tabla_de_pagina;
				pcb_to_exec->pagina_fault->numero_pagina = pagina->numero_pagina;
				free(pagina);
				return ERROR_MEMORIA;
			}
	    }
		else {
			// TLB HIT
			nro_marco = resultado_pedir_marco;
			// actualizar_pcb()
		} 

		desplazamiento_pagina = obtener_desplazamiento_pagina(dir_logica, tam_max_segmento, pagina_config->tamanio_pagina);
		direccion_fisica = nro_marco + desplazamiento_pagina;
		set_dir_fisica_a_instruccion(instruccion_a_decodificar, direccion_fisica);
	}
	free(pagina);
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

void set_valor_en_registro(int valor, t_pcb* pcb, char* parametro) {
	if(string_equals_ignore_case(parametro, "AX")) {
		pcb->registros.ax = valor;
	}
	else if(string_equals_ignore_case(parametro, "BX")) {
		pcb->registros.bx = valor;
	}
	else if(string_equals_ignore_case(parametro, "CX")) {
		pcb->registros.cx = valor;
	}
	else if(string_equals_ignore_case(parametro, "DX")) {
		pcb->registros.dx = valor;
	}
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
	log_debug(cpu_logger,"MOV_IN: finalmente laburo como una persona digna");
	int dir_fisica = atoi(parametro2);
	enviar_valor_con_codigo(dir_fisica, LEER, conexion_memoria);
	cod_mensaje cod_msj = recibir_operacion(conexion_memoria);
	if(cod_msj == OKI_LEER) {
		int valor = recibir_valor(conexion_memoria);
		log_debug(cpu_logger, "Se leyó el valor %d", valor);
		set_valor_en_registro(valor, pcb, parametro1);
	}
}

void ejecutar_mov_out(t_pcb* pcb, char* parametro1, char* parametro2) {
	log_debug(cpu_logger,"MOV_OUT: finalmente laburo como una persona digna");
	cod_mensaje mensaje = ESCRIBIR;
	int dir_fisica = atoi(parametro1);
	int valor = obtener_valor_del_registro(pcb, parametro2);
	//enviar_valor_con_codigo(dir_fisica, ESCRIBIR, conexion_memoria);
	enviar_datos(conexion_memoria, &mensaje, sizeof(mensaje));
	enviar_datos(conexion_memoria, &dir_fisica, sizeof(dir_fisica));
	enviar_datos(conexion_memoria, &valor, sizeof(valor));
	cod_mensaje cod_msj = recibir_operacion(conexion_memoria);
	if(cod_msj == OKI_ESCRIBIR) {
		log_debug(cpu_logger, "Se pudo escribir en memoria");
	}
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
		int cod_op = recibir_operacion(cliente_fd_dispatch);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cpu_logger, cliente_fd_dispatch);
			break;
		case PCB:
			t_pcb* pcb_to_exec;
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







							



