#ifndef CPU_INCLUDE_CPU_UTILS_H_
#define CPU_INCLUDE_CPU_UTILS_H_

#include <utils/comunicacion.h>
#include <utils/socket.h>
#include <utils/utiles_config.h>
#include <utils/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <tlb.h>
#include <mmu.h>

#define RUTA_LOGGER_CPU "./cpu.log"
#define RUTA_LOGGER_DEBUG_CPU "./cpu_db.log"
#define NOMBRE_MODULO "CPU"
#define RUTA_CPU_CONFIG "./src/cpu.config"


typedef struct {
	char* ip_cpu;
    char* ip_kernel;
	char* ip_memoria;
	char* puerto_memoria;
	char* puerto_escucha_dispatch;
	char* puerto_escucha_interrupt;
	int   retardo_intruccion;
	int   entradas_tlb;
    t_algoritmo reemplazo_tlb;
}t_cpu_config;

extern t_log *cpu_logger;
extern t_cpu_config* cpu_config;
extern int server_fd_dispatch;
extern int cliente_fd_dispatch;
extern int conexion_memoria;
extern bool interrupcion;
extern int instante_de_referencia_actual;
extern t_pagina_config* pagina_config;
extern t_list* tabla_tlb;
extern pthread_t th_kernel_dispatch;
extern pthread_t th_kernel_interrupt;

extern pthread_mutex_t interrupcion_mutex;

/* Configuracion y limpieza*/
void * configurar_cpu(t_config*);
void iniciar_conexion_con_memoria();
void cpu_config_destroy();
void esperar_conexiones();
void terminar_modulo();

/* Ciclo de instruccion */
void ciclo_de_instruccion_init();
void iniciar_ciclo_de_instruccion(t_pcb*);
instruccion* fetch(t_pcb*);
cod_operacion decode(t_pcb*, instruccion*);
void ejecutar_instruccion(t_pcb*, cod_operacion, instruccion*);
void ejecutar_set(t_pcb*, char*, char*);
void ejecutar_add(t_pcb*, char*, char*);
void ejecutar_mov_in(t_pcb*, char*, int);
void ejecutar_mov_out(t_pcb*, int, char*);

/* Retardo instruccion */
void valor_retardo_instruccion(int);

/* Conexiones con Kernel */
void* atender_kernel_dispatch(void*);
void* atender_kernel_interrupt(void*);

/* Traduccion direcciones logicas a fisicas*/
void set_dir_fisica_a_instruccion(instruccion*, int);
int obtener_dir_logica(instruccion*);
void set_valor_en_registro(int, t_pcb* , char*);

#endif

