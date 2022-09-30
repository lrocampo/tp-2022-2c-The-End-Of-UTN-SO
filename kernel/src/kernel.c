/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <kernel.h>

int main(void){
	pcbs = queue_create();
	//char* interrupcion = "interrupcion";
	char* dispatch = "dispatch";
	t_kernel_config* kernel_config;

	sem_init(&consolas,0,0);

	kernel_logger = iniciar_logger(RUTA_LOGGER_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	log_info(kernel_logger,"Arrancando kernel...\n");

	kernel_config = cargar_configuracion(RUTA_KERNEL_CONFIG, KERNEL);

	sem_init(&multiprogramacion,0,kernel_config->grado_multiprogramacion);
	
	kernel_server_fd = iniciar_servidor(kernel_config->ip_kernel, kernel_config->puerto_escucha);

	printf("Soy Kernel. Esperando conexion...\n");

	conexion_cpu_dispatch = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch);

	planificacion_init();

//////////////////////////////

	conexion_cpu_interrupt = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt);

	if(conexion_cpu_dispatch == -1 || conexion_cpu_interrupt == -1){
		return EXIT_FAILURE;
	}

	puts("Kernel conectado con cpu");

	enviar_mensaje(dispatch, conexion_cpu_dispatch);
	//enviar_mensaje(interrupcion, conexion_cpu_interrupt);

	liberar_conexion(conexion_cpu_dispatch);
	liberar_conexion(conexion_cpu_interrupt);
	
	puts("termino kernel\n");

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
			puts("Recibi el siguiente mensaje de una consola:");
			recibir_mensaje((int) (intptr_t) consola_fd);
			pcb = pcb_create();
			queue_push(pcbs,pcb);
			log_info(kernel_logger,"Se crea el proceso <PID> en NEW");
			sem_post(&consolas);
			break;
		case PAQUETE:
			instrucciones = deserializar_instrucciones(consola_fd);
			instruccion* ins = list_get(instrucciones,0);
			puts(string_itoa(ins->operacion));
			puts(ins->parametro1);
			puts(ins->parametro2);
			puts("ola");
			printf("tamanio lista %d",list_size(instrucciones));
			list_iterate(instrucciones, (void*) iterator);
			pcb = pcb_create();
			pcb->estado = NEW;
			queue_push(pcbs,pcb);
			break;
		case -1:
			return (void*) EXIT_FAILURE;
		default:
			puts("Termino kernel\n");
			break;
		}
	}
}

void* atender_cpu_dispatch(void* arg){
	while(1){
		// planificar
		sem_wait(&consolas);
		puts("estoy por mostrar el pcb");
		t_pcb* pcb = queue_pop(pcbs);
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
	
	puts("estoy por concetarme con el cpu");

	pthread_t thread_cpu_dispatch;
	
	pthread_create(&thread_cpu_dispatch, NULL, &atender_cpu_dispatch, NULL);

	puts("hilo creado");

	// Long-term scheduler
	while(1){

		int consola_fd = esperar_cliente(kernel_server_fd);
		puts("se conecto un cliente");
		pthread_t thread_consola;
		*p = consola_fd;
		pthread_create(&thread_consola, NULL, &atender_consola, /*(void*)*/ p);
		pthread_detach(thread_consola);

	}
}

void iterator(instruccion* value) {
	log_info(kernel_logger,"%d", value->operacion);
	log_info(kernel_logger,"%s", value->parametro1);
	log_info(kernel_logger,"%s", value->parametro2);
}