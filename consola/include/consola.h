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
	int segmentos[4];
	int tiempo_pantalla;
} t_consola_config;


t_log *consola_logger;
t_consola_config* consola_config;
int conexion_kernel;


void imprimir_por_pantalla();
int ingresar_por_teclado();
void* atender_solicitud_kernel();
void * configurar_consola(t_config*);
void terminar_modulo(char*, char*);
void consola_config_destroy();

#endif /* CONSOLA_INCLUDE_CONSOLA_H_ */
