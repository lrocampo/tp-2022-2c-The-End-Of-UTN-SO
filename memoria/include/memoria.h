/*
 * memoria.h
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_MEMORIA_H_
#define MEMORIA_INCLUDE_MEMORIA_H_

#include <stdio.h>
#include <utils/utiles_config.h>
#include <stdlib.h>
#include <utils/socket.h>
#include <pthread.h>
#include <utils/comunicacion.h>
#include "../../static/include/utils/logger.h"

#define RUTA_LOGGER_MEMORIA "./memoria.log"
#define RUTA_LOGGER_DEBUG_MEMORIA "./memoria_db.log"
#define NOMBRE_MODULO "MEMORIA"
#define RUTA_MEMORIA_CONFIG "./src/memoria.config"

t_log *memoria_logger;
t_memoria_config* memoria_config;
int server_fd_cpu;
int server_fd_kernel;
int cliente_kernel_fd;
int cliente_cpu_fd;
void* atender_pedido_de_memoria(void*);
void* atender_pedido_de_estructuras(void*);

#endif /* MEMORIA_INCLUDE_MEMORIA_H_ */
