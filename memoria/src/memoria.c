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

	memoria_server_fd = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_cpu);

	if(memoria_server_fd == -1){
		return EXIT_FAILURE;
	}

	pthread_t th_atender_pedido_de_memoria;
	pthread_create(&th_atender_pedido_de_memoria, NULL, &atender_pedido_de_memoria, NULL);
	pthread_detach(th_atender_pedido_de_memoria);

	while (1){

	}
	

	return EXIT_SUCCESS;
}

void* atender_pedido_de_memoria(void* args){
	cliente_cpu_fd = esperar_cliente(memoria_server_fd);
	log_debug(memoria_logger,"Se conecto un cliente a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_cpu_fd);
		if(mensaje == MENSAJE){
			recibir_mensaje(memoria_logger, cliente_cpu_fd);
			enviar_mensaje("Quien te conoce pa?", cliente_cpu_fd);
		}
		else {
			log_debug(memoria_logger,"Se desconecto el cliente.");
			exit(EXIT_FAILURE);
		}
	}
	
}