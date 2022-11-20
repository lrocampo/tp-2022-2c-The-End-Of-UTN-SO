#include <consola_utils.h>

t_log *consola_logger;
t_consola_config* consola_config;
t_proceso* proceso_consola;
pthread_t th_atender_solicitud_kernel;
int conexion_kernel;
char* ruta_config;
char* ruta_instrucciones;

/* Configuracion */

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
	log_destroy(consola_logger);
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

/* Solicitudes */

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

/* Utils */

void imprimir_por_pantalla(){
	int valor_a_imprimir = recibir_valor(conexion_kernel);
	ejecutar_espera(consola_config->tiempo_pantalla);
	printf("%d\n", valor_a_imprimir);
}

int ingresar_por_teclado(){
	int valor_ingresado;
	puts("Por favor, ingrese un valor");
	scanf("%d",&valor_ingresado);
	return valor_ingresado;
}