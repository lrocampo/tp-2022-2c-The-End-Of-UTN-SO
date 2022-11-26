/*
 * memoria.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <memoria.h>

void sighandler(int s)
{
	log_debug(memoria_logger, "Termino memoria\n");
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
	memoria_logger = iniciar_logger(RUTA_LOGGER_MEMORIA, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);

	/* LOGGER DE DEBUG */
	//memoria_logger = iniciar_logger(RUTA_LOGGER_DEBUG_MEMORIA, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(memoria_logger, "Arrancando memoria\n");

	memoria_config = cargar_configuracion(ruta_config, &configurar_memoria);

	free(ruta_config);

	memoria_principal_init();

	swap_init();

	solicitudes_a_memoria_init();

	esperar_conexiones();

	log_debug(memoria_logger, "Termino memoria\n");

	terminar_modulo();

	return EXIT_SUCCESS;
}