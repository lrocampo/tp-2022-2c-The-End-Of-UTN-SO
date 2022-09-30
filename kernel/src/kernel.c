/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <kernel.h>

// CONSOLA_FD PUEDE SER PPID

int main(void){
	cola_new_pcbs = queue_create();
	t_kernel_config* kernel_config;

	sem_init(&consolas,0,0);
	
	/* LOGGER DE ENTREGA */
	//kernel_logger = iniciar_logger(RUTA_LOGGER_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	
	/* LOGGER DE DEBUG */
	kernel_logger = iniciar_logger(RUTA_LOGGER_DEBUG_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(kernel_logger,"Arrancando kernel...");

	kernel_config = cargar_configuracion(RUTA_KERNEL_CONFIG, KERNEL);
	log_debug(kernel_logger,"Configuracion cargada correctamente");

	sem_init(&multiprogramacion,0,kernel_config->grado_multiprogramacion);
	
	kernel_server_fd = iniciar_servidor(kernel_config->ip_kernel, kernel_config->puerto_escucha);

	conexion_cpu_dispatch = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch);
	log_debug(kernel_logger,"Conexion creada correctamente con CPU DISPATCH");
	// TODO: conexion_cpu_interrupt = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt);

	planificacion_init();

	liberar_conexion(conexion_cpu_dispatch);
	// TODO: liberar_conexion(conexion_cpu_interrupt);
	
	log_debug(kernel_logger,"Termino Kernel");

	return EXIT_SUCCESS;
}

void* atender_consola(void* p){
	int consola_fd = *(int *) p;
	free(p);
	t_pcb* pcb;
	t_list* instrucciones = list_create();
		while(1){
		int cod_op = recibir_operacion(consola_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(kernel_logger, consola_fd);
			pcb = pcb_create();
			queue_push(cola_new_pcbs,pcb);
			log_info(kernel_logger,"Se crea el proceso <PID> en NEW");
			sem_post(&consolas);
			break;
		case PAQUETE_INSTRUCCIONES:
			instrucciones = recibir_paquete_con_funcion(consola_fd, deserializar_instruccion);//deserializar_instrucciones(consola_fd);
			log_debug(kernel_logger, "Recibí %d instrucciones", list_size(instrucciones));
			list_iterate(instrucciones, (void*) iterator);
			pcb = pcb_create();
			pcb->estado = NEW;
			queue_push(cola_new_pcbs,pcb);
			break;
		case -1:
			log_debug(kernel_logger, "Conexion con consola finalizada");
			return NULL;
		default:
			error_show("Operacion desconocida");
			break;
		}
	}
}

void* atender_cpu_dispatch(void* arg){
	while(1){
		// planificar
		sem_wait(&consolas);
		puts("estoy por mostrar el pcb");
		t_pcb* pcb = queue_pop(cola_new_pcbs);
		sem_wait(&multiprogramacion);
		pcb->estado = READY;
		
		log_info(kernel_logger,"Estado PCB: %d",pcb->estado);
		enviar_mensaje("dispatch", conexion_cpu_dispatch);

		// envias pcb elegido

		// recv (esperas pcb)

	}
}

void planificacion_init() {
	int *p = malloc(sizeof(int));
	
	pthread_t thread_cpu_dispatch;
	pthread_create(&thread_cpu_dispatch, NULL, &atender_cpu_dispatch, NULL);

	// Long-term scheduler
	while(1){
		log_debug(kernel_logger,"Soy Kernel. Esperando conexion...");
		int consola_fd = esperar_cliente(kernel_server_fd);
		log_debug(kernel_logger, "se conecto un cliente");
		pthread_t thread_consola;
		*p = consola_fd;
		pthread_create(&thread_consola, NULL, &atender_consola, p);
		pthread_detach(thread_consola);

	}
}

void iterator(instruccion* value) {
	log_debug(kernel_logger,"%d", value->operacion);
	log_debug(kernel_logger,"%s", value->parametro1);
	log_debug(kernel_logger,"%s", value->parametro2);
}