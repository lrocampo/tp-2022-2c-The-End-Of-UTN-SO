/*
 * memoria.h
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#ifndef MEMORIA_INCLUDE_MEMORIA_H_
#define MEMORIA_INCLUDE_MEMORIA_H_

#include <memoria_utils.h>

extern t_log *memoria_logger;
extern t_memoria_config* memoria_config;
extern int server_fd_cpu;
extern int server_fd_kernel;
extern int cliente_kernel_fd;
extern int cliente_cpu_fd;
extern t_list* lista_de_marcos; 
extern t_list* lista_de_tablas_de_pagina;

extern pthread_t th_atender_pedido_de_memoria;
extern pthread_t th_atender_pedido_de_estructuras;

#endif /* MEMORIA_INCLUDE_MEMORIA_H_ */
