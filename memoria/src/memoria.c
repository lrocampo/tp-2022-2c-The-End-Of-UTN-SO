/*
 * memoria.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <memoria.h>

int main(void){

	/* LOGGER DE ENTREGA */
	/* cpu_logger = iniciar_logger(RUTA_LOGGER_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_INFO); */

	/* LOGGER DE DEBUG */
	memoria_logger = iniciar_logger(RUTA_LOGGER_DEBUG_MEMORIA, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(memoria_logger,"Arrancando memoria\n");

	memoria_config = cargar_configuracion(RUTA_MEMORIA_CONFIG, MEMORIA);

	pthread_t th_atender_pedido_de_memoria;
	pthread_create(&th_atender_pedido_de_memoria, NULL, &atender_pedido_de_memoria, NULL);
	pthread_detach(th_atender_pedido_de_memoria);


	// al igual que con cpu creo otro hilo para atender el kernel

	pthread_t th_atender_pedido_de_estructuras;
	pthread_create(&th_atender_pedido_de_estructuras, NULL, &atender_pedido_de_estructuras, NULL);
	pthread_detach(th_atender_pedido_de_estructuras);
	while (1){

	}

	
	return EXIT_SUCCESS;
}

void* atender_pedido_de_memoria(void* args){
	server_fd_cpu = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_cpu);
	if(server_fd_cpu == -1){
		pthread_exit(NULL);
	}
	cliente_cpu_fd = esperar_cliente(server_fd_cpu);
	log_debug(memoria_logger,"Se conecto un CPU a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_cpu_fd);
		if(mensaje == MENSAJE){
			recibir_mensaje(memoria_logger, cliente_cpu_fd);
			enviar_mensaje("Quien te conoce pa?", cliente_cpu_fd);
		}
		else {
			log_debug(memoria_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
		}
	}
	
}

// todo mauro
void* atender_pedido_de_estructuras(void* args) {
	server_fd_kernel = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_kernel);
	if(server_fd_kernel == -1){
		pthread_exit(NULL);
	}
 	cliente_kernel_fd = esperar_cliente(server_fd_kernel);
 	log_debug(memoria_logger,"Se conecto un Kernel a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_kernel_fd);
 		if(mensaje == MENSAJE){
 			recibir_mensaje(memoria_logger, cliente_kernel_fd);
 			enviar_mensaje("Atiendo boludos", cliente_kernel_fd);
 		}
 		else {
 			log_debug(memoria_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
 		}
 	}
 }