#ifndef CONSOLA_INCLUDE_CONSOLA_UTILS_H_
#define CONSOLA_INCLUDE_CONSOLA_UTILS_H_

#include <utils/comunicacion.h>
#include <utils/socket.h>
#include <utils/utiles_config.h>
#include <utils/logger.h>
#include <commons/string.h>
#include <pthread.h>
#include <consola_parser.h>
#include <stdio.h>
#include <consola.h>

typedef struct {
	char* ip;
	char* puerto;
	t_list* segmentos;
	int tiempo_pantalla;
} t_consola_config;

extern t_log *consola_logger;
extern t_consola_config* consola_config;
extern t_proceso* proceso_consola;
extern pthread_t th_atender_solicitud_kernel;
extern int conexion_kernel;
extern char* ruta_config;
extern char* ruta_instrucciones;

void imprimir_por_pantalla();
int ingresar_por_teclado();
void* atender_solicitud_kernel();
t_list* config_get_segmentos_list(t_config*);
void * configurar_consola(t_config*);
void terminar_modulo();
void consola_config_destroy();

#endif /* CONSOLA_INCLUDE_CONSOLA_UTILS_H_ */