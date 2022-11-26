/*
 * consola.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <consola.h>

// ./consola.out ruta/del/config ruta/de/las/instrucciones

void sighandler(int s)
{
	log_debug(consola_logger, "termino consola");
	terminar_modulo();
	exit(0);
}

int main(int argc, char **argv)
{

	signal(SIGINT, sighandler);

	// TODO: achicar en funciones

	if (argc != 3)
	{
		error_show("Argumentos invalidos!");
		return EXIT_FAILURE;
	}

	ruta_config = strdup(argv[1]);
	ruta_instrucciones = strdup(argv[2]);
	char *instrucciones_string;
	t_list *instrucciones;

	/* LOGGER DE ENTREGA */
	consola_logger = iniciar_logger(RUTA_LOGGER_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);

	/* LOGGER DE DEBUG */
	//consola_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);
	log_debug(consola_logger, "Arrancando consola...");

	consola_config = cargar_configuracion(ruta_config, configurar_consola);
	log_debug(consola_logger, "Configuracion cargada correctamente");

	conexion_kernel = crear_conexion(consola_config->ip, consola_config->puerto);
	log_debug(consola_logger, "Conexion creada correctamente");

	instrucciones_string = leer_archivo_pseudocodigo(ruta_instrucciones);
	log_debug(consola_logger, "Archivo de pseudocodigo leido correctamente");

	instrucciones = obtener_pseudocodigo(instrucciones_string);
	log_debug(consola_logger, "Instrucciones parseadas correctamente");

	proceso_consola = proceso_create(instrucciones, consola_config->segmentos);

	enviar_proceso(proceso_consola, conexion_kernel);
	log_debug(consola_logger, "Instrucciones enviadas");

	proceso_destroy(proceso_consola);

	pthread_create(&th_atender_solicitud_kernel, NULL, &atender_solicitud_kernel, NULL);
	pthread_join(th_atender_solicitud_kernel, NULL);

	log_debug(consola_logger, "termino consola");

	terminar_modulo();

	return EXIT_SUCCESS;
}