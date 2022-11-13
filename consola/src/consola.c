/*
 * consola.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <consola.h>

// ./consola.out ruta/del/config ruta/de/las/instrucciones  

void sighandler(int s){
    terminar_modulo();
    exit(0);
}

int main(int argc, char **argv) {

	signal(SIGINT, sighandler);

	// TODO: achicar en funciones

	if(argc != 3){ 
		error_show("Argumentos invalidos!");
		return EXIT_FAILURE;
	}

	ruta_config = strdup(argv[1]); 
	ruta_instrucciones = strdup(argv[2]);
	char* instrucciones_string;
	t_list* instrucciones;

	/* LOGGER DE ENTREGA */
	//consola_logger = iniciar_logger(RUTA_LOGGER_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	
	/* LOGGER DE DEBUG */
	consola_logger = iniciar_logger(RUTA_LOGGER_DEBUG_CONSOLA, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);
	log_debug(consola_logger,"Arrancando consola...");

	consola_config = cargar_configuracion(ruta_config, configurar_consola);
	log_debug(consola_logger,"Configuracion cargada correctamente");

	conexion_kernel = crear_conexion(consola_config->ip, consola_config->puerto);
	log_debug(consola_logger,"Conexion creada correctamente");

	instrucciones_string = leer_archivo_pseudocodigo(ruta_instrucciones);
	log_debug(consola_logger,"Archivo de pseudocodigo leido correctamente");

	instrucciones = obtener_pseudocodigo(instrucciones_string);
	log_debug(consola_logger,"Instrucciones parseadas correctamente");

	proceso_consola = proceso_create(instrucciones, consola_config->segmentos);

	enviar_proceso(proceso_consola, conexion_kernel);
	log_debug(consola_logger,"Instrucciones enviadas");

	pthread_create(&th_atender_solicitud_kernel, NULL, &atender_solicitud_kernel, NULL);
	pthread_join(th_atender_solicitud_kernel, NULL); 

	log_debug(consola_logger, "termino consola"); 

	terminar_modulo();

	return EXIT_SUCCESS;
}

void* atender_solicitud_kernel(){
	while(1){
		cod_mensaje cod_msj = recibir_operacion(conexion_kernel);
		
		switch (cod_msj)
		{
			case FINALIZAR:
				log_debug(consola_logger, "Ha llegado mi hora");
				pthread_exit(NULL);
				break;
			case TECLADO:
				int valor_ingresado = ingresar_por_teclado();
				enviar_valor_ingresado(valor_ingresado, conexion_kernel);
				break;
			case PANTALLA:
				cod_msj = OKI_PANTALLA;
				imprimir_por_pantalla();
				ejecutar_espera(consola_config->tiempo_pantalla);
				enviar_datos(conexion_kernel,&cod_msj,sizeof(cod_msj));
				break;
			default:
				// TODO: notificacion con datos error
				error_show("Error, mensaje desconocido.\n");
				pthread_exit(NULL);
				break;
		}
	}
}

void imprimir_por_pantalla(){
	int valor_a_imprimir = recibir_valor(conexion_kernel);
	printf("%d\n", valor_a_imprimir);
}

int ingresar_por_teclado(){
	int valor_ingresado;
	puts("Por favor, ingrese un valor");
	scanf("%d",&valor_ingresado);
	return valor_ingresado;
}

t_proceso* proceso_create(t_list* instrucciones, t_list* segmentos){
	t_proceso* proceso = malloc(sizeof(t_proceso));
	proceso->instrucciones = instrucciones;
	proceso->segmentos = segmentos;
	return proceso;
}

void * configurar_consola(t_config* config){
	t_consola_config* consola_config;
	consola_config = malloc(sizeof(t_consola_config));
	consola_config->ip = strdup(config_get_string_value(config, "IP"));
	consola_config->puerto = strdup(config_get_string_value(config, "PUERTO"));
	consola_config->tiempo_pantalla = config_get_int_value(config, "TIEMPO_PANTALLA");
	consola_config->segmentos = config_get_segmentos_list(config);
	return consola_config;
}

void terminar_modulo(){
	liberar_conexion(conexion_kernel);
	free(ruta_config);
	free(ruta_instrucciones);
	consola_config_destroy();
}

void consola_config_destroy(){
	free(consola_config->ip);
	free(consola_config->puerto);
	free(consola_config);
}

t_list* config_get_segmentos_list(t_config* config){
	t_list* lista_segmentos = list_create();
	char** segmentos_array = config_get_array_value(config, "SEGMENTOS");
	for(int i = 0; i < string_array_size(segmentos_array); i++ ){
		list_add(lista_segmentos, segmentos_array[i]);
	}
	free(segmentos_array);
	return lista_segmentos;
}