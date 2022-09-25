/*
 * memoria.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <memoria.h>

int main(void){

	memoria_logger = iniciar_logger(RUTA_LOGGER_MEMORIA, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	log_info(memoria_logger,"Arrancando memoria...\n");

	printf("memoria\n");

	return 0;
}
