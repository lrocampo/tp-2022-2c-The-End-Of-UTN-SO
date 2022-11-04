/*
 * cpu.h
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#ifndef CPU_INCLUDE_CPU_H_
#define CPU_INCLUDE_CPU_H_

#include <utils/comunicacion.h>
#include <utils/socket.h>
#include <utils/utiles_config.h>
#include <utils/logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

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
}t_cpu_config;

t_log *cpu_logger;
t_cpu_config* cpu_config;
int server_fd_dispatch;
int cliente_fd_dispatch;
int conexion_memoria;
bool interrupcion;

pthread_mutex_t interrupcion_mutex;

// Contexto de ejecucion
registros_de_proposito_general registros_cpu;

void iniciar_ciclo_de_instruccion(t_pcb*);
void* atender_kernel_interrupt(void*);
instruccion* fetch(t_pcb*);
cod_operacion decode(instruccion*);
void ejecutar_instruccion(t_pcb*, cod_operacion, instruccion*);
void ejecutar_set(t_pcb*, char*, char*);
void ejecutar_add(t_pcb*, char*, char*);
void ejecutar_mov_in(t_pcb*, char*, char*);
void ejecutar_mov_out(t_pcb*, char*, char*);
void ejecutar_io(t_pcb*, char*, char*);
uint32_t obtener_valor_del_registro(t_pcb*, char*);
void iniciar_conexion_con_memoria();
void * configurar_cpu(t_config*);
void cpu_config_destroy();
void terminar_modulo();

#endif /* CPU_INCLUDE_CPU_H_ */
