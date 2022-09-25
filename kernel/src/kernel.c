/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <kernel.h>

int main(void){
	pcbs = list_create();
	char* interrupcion = "interrupcion";
	char* dispatch = "dispatch";
	t_kernel_config* kernel_config;

	sem_init(&procesos,0,1);

	kernel_logger = iniciar_logger(RUTA_LOGGER_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	log_info(kernel_logger,"Arrancando kernel...\n");

	kernel_config = cargar_configuracion(RUTA_KERNEL_CONFIG, KERNEL);
	
	int server_fd = iniciar_servidor(kernel_config->ip_kernel, kernel_config->puerto_escucha);

	printf("Soy Kernel. Esperando conexion...\n");

	conexion_cpu_dispatch = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch);


	pthread_t thread_cpu_dispatch;
	
	pthread_create(&thread_cpu_dispatch, NULL, &atender_cpu_dispatch, NULL);

	while(1){

	int cliente_fd = esperar_cliente(server_fd);
	pthread_t thread_consola;
	
	pthread_create(&thread_consola, NULL, &atender_consola, (void*) cliente_fd);
	pthread_detach(thread_consola);

	}


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

void* atender_consola(void* cliente_fd){
		while(1){
		int cod_op = recibir_operacion((int) cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			puts("Recibi el siguiente mensaje de una consola:");
			recibir_mensaje((int) cliente_fd);
			list_add(pcbs,"consola");
			sem_post(&procesos);
			break;
		case -1:
			return EXIT_FAILURE;
		default:
			puts("Termino kernel\n");
			break;
		}
	}
}

void* atender_cpu_dispatch(void* arg){
	while(1){
		// planificar
		sem_wait(&procesos);
		enviar_mensaje("dispatch", conexion_cpu_dispatch);

		// envias pcb elegido

		// recv (esperas pcb)

	}
}