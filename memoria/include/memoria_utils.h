#ifndef MEMORIA_INCLUDE_MEMORIA_UTILS_H_
#define MEMORIA_INCLUDE_MEMORIA_UTILS_H_

#include <stdio.h>
#include <utils/utiles_config.h>
#include <stdlib.h>
#include <utils/socket.h>
#include <swap.h>
#include <esquema.h>
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
	char* path_swap; 
	int tamanio_memoria;
	int tamanio_pagina;
	int entradas_por_tabla;
	/* int retardo_memoria; */
	/* char* algoritmo_reemplazo; */
	int marcos_por_proceso;
	int retardo_swap;
	int tamanio_swap;
}t_memoria_config;

extern t_log *memoria_logger;
extern t_memoria_config* memoria_config;

extern int memoria_server_cpu_fd;
extern int memoria_server_kernel_fd;
extern int cliente_kernel_fd;
extern int cliente_cpu_fd;

extern t_list* lista_de_marcos;
extern t_list* lista_de_marcos_swap;
extern t_list* lista_de_tablas_de_paginas;  

extern void* espacio_memoria;
extern void* swap;

extern pthread_t th_atender_cpu;
extern pthread_t th_atender_kernel;

extern pthread_mutex_t memoria_swap_mutex;
extern pthread_mutex_t memoria_usuario_mutex;
extern pthread_mutex_t lista_de_tablas_de_paginas_mutex;
extern pthread_mutex_t lista_de_tablas_de_paginas_swap_mutex;

void memoria_principal_init();
void marcos_memoria_principal_init();
t_marco* obtener_marco_libre(t_list*);
int obtener_marco_libre_memoria();
void marcos_init(t_list*, int, int);
void algoritmo_init();
void solicitudes_a_memoria_init();
void* atender_cpu(void*);
void* atender_kernel(void*);
int obtener_numero_de_marco(t_pagina*);
void * configurar_memoria(t_config*);
void esperar_conexiones();
void terminar_modulo();
void escribir_en_memoria(void*, void*);
void memoria_config_destroy();
bool marco_libre(t_marco*);
void escribir_en_memoria_principal(int, int*);
int leer_en_memoria_principal(int);

#endif /* MEMORIA_INCLUDE_MEMORIA_UTILS_H_ */