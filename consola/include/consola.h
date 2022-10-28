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
#include <consola_parser.h>
#include <stdio.h>

#define RUTA_LOGGER_CONSOLA "./consola.log"
#define RUTA_LOGGER_DEBUG_CONSOLA "./consola_db.log"
#define NOMBRE_MODULO "Consola"

t_log *consola_logger;
t_consola_config* consola_config;
int kernel_fd;

#endif /* CONSOLA_INCLUDE_CONSOLA_H_ */
