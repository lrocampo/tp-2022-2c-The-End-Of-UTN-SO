/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <kernel.h>

void sighandler(int s)
{
	log_debug(kernel_logger, "Termino Kernel");
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
	kernel_logger = iniciar_logger(RUTA_LOGGER_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);

	/* LOGGER DE DEBUG */
	//kernel_logger = iniciar_logger(RUTA_LOGGER_DEBUG_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(kernel_logger, "Arrancando kernel...");

	kernel_config = cargar_configuracion(ruta_config, configurar_kernel);
	log_debug(kernel_logger, "Configuracion cargada correctamente");

	free(ruta_config);

	kernel_server_fd = iniciar_servidor(kernel_config->ip_kernel, kernel_config->puerto_escucha);

	iniciar_conexiones_con_cpu();

	iniciar_conexion_con_memoria();

	planificacion_init();

	esperar_conexiones();

	return EXIT_SUCCESS;
}