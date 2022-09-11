/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include "../include/kernel.h"

int main(void){

	int server_fd = iniciar_servidor();
	int cliente_fd = esperar_cliente(server_fd);
	printf("Kernel\n");


	while(1){
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case -1:
			return EXIT_FAILURE;
		default:
			break;
		}
	}

	puts("termino kernel\n");

	return EXIT_SUCCESS;
}

