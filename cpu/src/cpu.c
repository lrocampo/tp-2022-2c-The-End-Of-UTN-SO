/*
 * cpu.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */
#include <cpu.h>

int main(void){
	//t_cpu_config* cpu_config;

	/* LOGGER DE ENTREGA */
	/* cpu_logger = iniciar_logger(RUTA_LOGGER_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_INFO); */

	/* LOGGER DE DEBUG */
	cpu_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(cpu_logger,"Arrancando cpu");

	cpu_config = cargar_configuracion(RUTA_CPU_CONFIG, configurar_cpu);
	log_debug(cpu_logger,"Configuracion cargada correctamente");

	iniciar_conexion_con_memoria();

	ciclo_de_instruccion_init();

	esperar_conexiones();

	log_debug(cpu_logger,"termino cpu\n");

	terminar_modulo();

	return EXIT_SUCCESS;
}