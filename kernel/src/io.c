#include <io.h>

/* IO */

void dispositivos_io_init(){
	s_dispositivos_io = malloc(sizeof(sem_t) * cantidad_dispositivos);
	cola_dispositivo_mutex = malloc(sizeof(pthread_mutex_t) * cantidad_dispositivos);

	for(int i = 0; i < cantidad_dispositivos; i++){
		t_dispositivo* dispositivo = list_get(dispositivos_io, i);
		sem_init(&s_dispositivos_io[i], 0, 0);
		pthread_mutex_init(&cola_dispositivo_mutex[i], NULL);
		pthread_t th_dispositivo_io;
		pthread_create(&th_dispositivo_io, NULL, &ejecucion_io, (void*) dispositivo);
		pthread_detach(th_dispositivo_io);
	}
}

void solicitar_io(t_pcb* pcb, instruccion* ultima_instruccion) {
	char* dispositivo = ultima_instruccion->parametro1;
	cambiar_estado(pcb, BLOCK);

	if(string_equals_ignore_case(dispositivo, "TECLADO") || string_equals_ignore_case(dispositivo, "PANTALLA")) {
		pthread_t th_solicitud_consola;
        pthread_create(&th_solicitud_consola, NULL, &solicitar_io_consola, (void*)pcb);
        pthread_detach(th_solicitud_consola);
	}
	else {
		solicitar_dispositivo(pcb, ultima_instruccion);
	}
}

void solicitar_dispositivo(t_pcb* pcb, instruccion* ultima_instruccion){
	t_dispositivo* dispositivo = obtener_dispositivo_por_nombre(ultima_instruccion->parametro1);
	safe_pcb_push(dispositivo->cola, pcb, &cola_dispositivo_mutex[dispositivo->indice]);
	sem_post(&s_dispositivos_io[dispositivo->indice]);	
}

t_dispositivo* obtener_dispositivo_por_nombre(char* nombre){
	int i = 0;
	t_dispositivo* dispositivo = list_get(dispositivos_io, i); 
	while(!string_equals_ignore_case(dispositivo->nombre, nombre) && i < cantidad_dispositivos){
		dispositivo = list_get(dispositivos_io, i);
		i++; 
	}
	return dispositivo;
}

void* ejecucion_io(void* arg){
	t_dispositivo* dispositivo = (t_dispositivo*) arg;
	int idx = dispositivo->indice;
	t_queue* cola_de_atencion = dispositivo->cola;
	while(1){
		sem_wait(&s_dispositivos_io[idx]);
		t_pcb* pcb = safe_pcb_pop(cola_de_atencion, &cola_dispositivo_mutex[idx]);
		log_info(kernel_logger, "PID: %d - Bloqueado por: %s", pcb->pid, dispositivo->nombre);
		instruccion* ultima_instruccion = obtener_ultima_instruccion(pcb);
		int unidades_tiempo = (atoi(ultima_instruccion->parametro2) * dispositivo->duracion);
		ejecutar_espera(unidades_tiempo);
		log_debug(kernel_logger, "Tiempo esperado: %d", unidades_tiempo);
		pasar_a_ready(pcb);
	}
}

void* solicitar_io_consola(void *arg){
	t_pcb *pcb = (t_pcb *) arg;
	instruccion *instruccionIO = obtener_ultima_instruccion(pcb);
	solicitud(instruccionIO, pcb);
	pasar_a_ready(pcb);
	pthread_exit(NULL);
}

void solicitud(instruccion* instruccionIO, t_pcb *pcb) {
	cod_mensaje cod_msj;
	char* dispositivo = instruccionIO->parametro1;
	char *registro = instruccionIO->parametro2;
	int consola_fd =  pcb->socket_consola;
	log_info(kernel_logger, "PID: %d - Bloqueado por: %s", pcb->pid, dispositivo);
	if(string_equals_ignore_case(dispositivo,"TECLADO")) {
		cod_msj = TECLADO;
		enviar_datos(consola_fd,&cod_msj,sizeof(cod_msj));
		cod_mensaje codigo = recibir_operacion(consola_fd);
		if(codigo == OKI_TECLADO){
			char* valorToString = recibir_valor_string(consola_fd);
			set_valor_registro(pcb,registro,valorToString);
			free(valorToString);
		}else{
			log_debug(kernel_logger,"ERROR");
		}
	}
     else {
		uint32_t valor_registro = obtener_valor_del_registro(pcb,registro);
        enviar_valor_a_imprimir((int)valor_registro,consola_fd);
		cod_mensaje codigo = recibir_operacion(consola_fd);
		 if(codigo == OKI_PANTALLA){
			log_debug(kernel_logger,"Todo Bien, Imprimir por Pantalla");
		 }else{
			log_debug(kernel_logger,"ERROR");
		 }
    	}
}