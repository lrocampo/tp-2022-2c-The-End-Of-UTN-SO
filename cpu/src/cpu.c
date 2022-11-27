/*
 * cpu.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */
#include <cpu.h>

void sighandler(int s)
{
	log_debug(cpu_logger, "termino cpu\n");
	terminar_modulo();
	exit(0);
}

int main(int argc, char **argv)
{

	signal(SIGINT, sighandler);

	if (argc != 2)
	{
		error_show("Error de argumentos");
		return EXIT_FAILURE;
	}

	char *ruta_config = strdup(argv[1]);

	/* LOGGER DE ENTREGA */
	cpu_logger = iniciar_logger(RUTA_LOGGER_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);

	/* LOGGER DE DEBUG */
	// cpu_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CPU, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(cpu_logger, "Arrancando cpu");

	cpu_config = cargar_configuracion(ruta_config, configurar_cpu);
	log_debug(cpu_logger, "Configuracion cargada correctamente");

	free(ruta_config);

	iniciar_conexion_con_memoria();

	ciclo_de_instruccion_init();

	esperar_conexiones();

	return EXIT_SUCCESS;
}