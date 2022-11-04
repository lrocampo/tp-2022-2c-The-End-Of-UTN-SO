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

typedef struct {
	char* ip_memoria;
	char* ip_cpu;
    char* ip_kernel;
	char* puerto_escucha_cpu;
	char* puerto_escucha_kernel;
	/* int tamanio_memoria; */
	/* int tamanio_pagina; */
	/* int entradas_por_pagina; */
	/* int retardo_memoria; */
	/* char* algoritmo_reemplazo; */
	/* int* marcos_por_proceso; */
	/* int* retardo_swap; */
	/* int* path_swap; */
	/* int* tamanio_swap; */
}t_memoria_config;

t_log *memoria_logger;
t_memoria_config* memoria_config;
int server_fd_cpu;
int server_fd_kernel;
int cliente_kernel_fd;
int cliente_cpu_fd;
void* atender_pedido_de_memoria(void*);
void* atender_pedido_de_estructuras(void*);
void * configurar_memoria(t_config*);
void terminar_modulo();
void memoria_config_destroy();

#endif /* MEMORIA_INCLUDE_MEMORIA_H_ */
