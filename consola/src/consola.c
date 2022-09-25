/*
 * consola.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <consola.h>
#include <consola_parser.h>

// ./consola.out ruta/del/config ruta/de/las/instrucciones  

int main(int argc, char **argv){

	// una vez que tengamos el archivo de instrucciones, seria != 3. Por ahora lo dejamos asi:
	if(argc != 2){ 
		puts("Argumentos invalidos!");
		return EXIT_FAILURE;
	}

	t_consola_config* consola_config;

	char *ruta_config = strdup(argv[1]); 

	consola_logger = iniciar_logger(RUTA_LOGGER_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	log_info(consola_logger,"Arrancando consola...\n");

	consola_config = cargar_configuracion(ruta_config, CONSOLA);

	int conexion;
	char* valor = "consola";

	conexion = crear_conexion(consola_config->ip, consola_config->puerto);

	enviar_mensaje(valor, conexion);
	sleep(20);	
	printf("Soy consola. Envie el siguiente mensaje a kernel: %s\n", valor);
	liberar_conexion(conexion);
	puts("termino consola\n");


	return 0;
}
