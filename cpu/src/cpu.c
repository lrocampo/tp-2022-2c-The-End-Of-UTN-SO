/*
 * cpu.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */
#include <cpu.h>

int main(void){
	t_cpu_config* cpu_config;

	/* LOGGER DE ENTREGA */
	cpu_logger = iniciar_logger(RUTA_LOGGER_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);

	/* LOGGER DE DEBUG */
	cpu_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(cpu_logger,"Arrancando cpu");

	cpu_config = cargar_configuracion(RUTA_CPU_CONFIG, CPU);
	log_debug(cpu_logger,"Configuracion cargada correctamente");

	int server_fd_dispatch = iniciar_servidor(cpu_config->ip_cpu, cpu_config->puerto_escucha_dispatch);

	int cliente_fd_dispatch = esperar_cliente(server_fd_dispatch);
	log_debug(cpu_logger,"Se conecto un cliente a DISPATCH");
	

	if(cliente_fd_dispatch == -1){
		return EXIT_FAILURE;
	}

	// en otro hilo:
	//int server_fd_interrupt = iniciar_servidor(ip_cpu, puerto_cpu_interrupt);
	//int cliente_fd_interrupt = esperar_cliente(server_fd_interrupt);

	/*if(cliente_fd_interrupt == -1){
			return EXIT_FAILURE;
		}
	*/

	while(1){
			int cod_op = recibir_operacion(cliente_fd_dispatch);
			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cpu_logger, cliente_fd_dispatch);
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

	return EXIT_SUCCESS;
}


