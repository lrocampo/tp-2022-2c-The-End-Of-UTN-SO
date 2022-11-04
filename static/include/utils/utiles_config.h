#ifndef INCLUDE_UTILS_UTILES_CONFIG_H_
#define INCLUDE_UTILS_UTILES_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/error.h>
#include <string.h>
#include <utils/contexto.h>

typedef struct {
	char* ip;
	char* puerto;
	int segmentos[4];
	int tiempo_pantalla;
} t_consola_config;

typedef struct {
	char* ip_cpu;
    char* ip_kernel;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
	int   retardo_intruccion;
}t_cpu_config;

typedef struct {
	char* ip_cpu;
    char* ip_kernel;
	char* ip_memoria;
    char* puerto_escucha;
    char* puerto_memoria;
	char* puerto_cpu_dispatch;
	char* puerto_cpu_interrupt;
	int grado_multiprogramacion;
	t_algoritmo algoritmo;
	int quantum_RR;
	t_list* dispositivos_io;
} t_kernel_config;

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

typedef enum {
	KERNEL,CPU, MEMORIA,CONSOLA
} t_tipo_archivo;


void* cargar_configuracion(char* path_archivo, t_tipo_archivo tipo_archivo);

#endif /* INCLUDE_UTILS_UTILES_CONFIG_H_ */
