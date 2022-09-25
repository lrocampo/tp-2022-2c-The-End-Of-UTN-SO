/*
 * cpu.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */
#include "../include/cpu.h"

int main(void){
	t_cpu_config* cpu_config;

	cpu_logger = iniciar_logger(RUTA_LOGGER_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	log_info(cpu_logger,"Arrancando cpu...\n");

	cpu_config = cargar_configuracion(RUTA_CPU_CONFIG, CPU);

	int server_fd_dispatch = iniciar_servidor(cpu_config->ip_cpu, cpu_config->puerto_escucha_dispatch);

	int cliente_fd_dispatch = esperar_cliente(server_fd_dispatch);

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

	puts("se conecto kernel a dispatch");

	while(1){
			int cod_op = recibir_operacion(cliente_fd_dispatch);
			switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(cliente_fd_dispatch);
				break;
			case -1:
				break;
			default:
				break;
			}
			break;
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
	puts("termino cpu\n");

	return 0;
}


