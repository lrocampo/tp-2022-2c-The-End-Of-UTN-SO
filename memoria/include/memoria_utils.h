#ifndef MEMORIA_INCLUDE_MEMORIA_UTILS_H_
#define MEMORIA_INCLUDE_MEMORIA_UTILS_H_

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

extern t_log *memoria_logger;
extern t_memoria_config* memoria_config;
extern int server_fd_cpu;
extern int server_fd_kernel;
extern int cliente_kernel_fd;
extern int cliente_cpu_fd;

extern pthread_t th_atender_pedido_de_memoria;
extern pthread_t th_atender_pedido_de_estructuras;

void solicitudes_a_memoria_init();
void* atender_pedido_de_memoria(void*);
void* atender_pedido_de_estructuras(void*);
void * configurar_memoria(t_config*);
void esperar_conexiones();
void terminar_modulo();
void memoria_config_destroy();

#endif