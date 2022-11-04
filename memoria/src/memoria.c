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

	memoria_config = cargar_configuracion(RUTA_MEMORIA_CONFIG, &configurar_memoria);

	solicitudes_a_memoria_init();

	esperar_conexiones();

	log_debug(memoria_logger, "Termino memoria\n");

	terminar_modulo();
	
	return EXIT_SUCCESS;
}