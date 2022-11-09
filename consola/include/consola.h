/*
 * consola.h
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#ifndef CONSOLA_INCLUDE_CONSOLA_H_
#define CONSOLA_INCLUDE_CONSOLA_H_

#include <utils/comunicacion.h>
#include <utils/socket.h>
#include <utils/utiles_config.h>
#include <utils/logger.h>
#include <commons/string.h>
#include <pthread.h>
#include <consola_parser.h>
#include <stdio.h>

#define RUTA_LOGGER_CONSOLA "./consola.log"
#define RUTA_LOGGER_DEBUG_CONSOLA "./consola_db.log"
#define NOMBRE_MODULO "Consola"

typedef struct {
	char* ip;
	char* puerto;
	t_list* segmentos;
	int tiempo_pantalla;
} t_consola_config;


t_log *consola_logger;
t_consola_config* consola_config;
t_proceso* proceso_consola;
int conexion_kernel;


void imprimir_por_pantalla();
int ingresar_por_teclado();
t_proceso* crear_proceso(t_list*, char**);
void* atender_solicitud_kernel();
t_list* config_get_segmentos_list(t_config*);
void * configurar_consola(t_config*);
void terminar_modulo(char*, char*);
t_proceso* proceso_create(t_list*, t_list*);
void consola_config_destroy();

#endif /* CONSOLA_INCLUDE_CONSOLA_H_ */
