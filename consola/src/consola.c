/*
 * consola.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <consola.h>

// ./consola.out ruta/del/config ruta/de/las/instrucciones  

int main(int argc, char **argv) {

	if(argc != 3){ 
		error_show("Argumentos invalidos!");
		return EXIT_FAILURE;
	}

	char* ruta_config = strdup(argv[1]); 
	char* ruta_instrucciones = strdup(argv[2]);
	t_list* instrucciones;

	/* LOGGER DE ENTREGA */
	//consola_logger = iniciar_logger(RUTA_LOGGER_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	
	/* LOGGER DE DEBUG */
	consola_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);
	log_debug(consola_logger,"Arrancando consola...");

	consola_config = cargar_configuracion(ruta_config, CONSOLA);
	log_debug(consola_logger,"Configuracion cargada correctamente");

	kernel_fd = crear_conexion(consola_config->ip, consola_config->puerto);
	log_debug(consola_logger,"Conexion creada correctamente");

	char *instrucciones_string = leer_archivo_pseudocodigo(ruta_instrucciones);
	log_debug(consola_logger,"Archivo de pseudocodigo leido correctamente");

	instrucciones = obtener_pseudocodigo(instrucciones_string);
	log_debug(consola_logger,"Instrucciones parseadas correctamente");

	enviar_instrucciones(instrucciones, kernel_fd);
	log_debug(consola_logger,"Instrucciones enviadas");

	liberar_conexion(kernel_fd);

	log_debug(consola_logger, "termino consola"); 

	free(ruta_config);
	free(ruta_instrucciones);
	free(instrucciones_string);

	return EXIT_SUCCESS;
}

