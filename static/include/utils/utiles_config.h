#ifndef INCLUDE_UTILS_UTILES_CONFIG_H_
#define INCLUDE_UTILS_UTILES_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/error.h>
#include <string.h>

typedef struct {
	char* ip_cpu;
    char* ip_kernel;
    char* puerto_escucha;
	char* puerto_cpu_dispatch;
	char* puerto_cpu_interrupt;
	int grado_multiprogramacion;
} t_kernel_config;

typedef struct {
	char* ip;
	char* puerto;
	int segmentos[4];
} t_consola_config;

typedef struct {
	char* ip_cpu;
    char* ip_kernel;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
}t_cpu_config;

typedef enum {
	KERNEL,CPU, MEMORIA,CONSOLA
} t_tipo_archivo;


void* cargar_configuracion(char* path_archivo, t_tipo_archivo tipo_archivo);

#endif /* INCLUDE_UTILS_UTILES_CONFIG_H_ */
