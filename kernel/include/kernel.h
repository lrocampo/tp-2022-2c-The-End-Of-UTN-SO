/*
 * kernel.h
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#ifndef KERNEL_INCLUDE_KERNEL_H_
#define KERNEL_INCLUDE_KERNEL_H_

#include <utils/comunicacion.h>
#include <utils/socket.h>
#include <utils/logger.h>
#include <utils/utiles_config.h>
#include <utils/contexto.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#define RUTA_LOGGER_KERNEL "./kernel.log"
#define RUTA_LOGGER_DEBUG_KERNEL "./kernel_db.log"
#define NOMBRE_MODULO "Kernel"
#define RUTA_KERNEL_CONFIG "./src/kernel.config"

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;

t_log* kernel_logger;

t_queue* cola_new_pcbs;
t_queue* cola_exit_pcbs;
t_queue* cola_ready_pcbs;

sem_t consolas;
sem_t multiprogramacion;
sem_t procesos_finalizados; // existe alguna convencion para el nombre de los semaforos de sincro?
uint32_t pid_actual;
pthread_mutex_t pid_mutex;
pthread_mutex_t cola_new_pcbs_mutex;
int kernel_server_fd;

void planificacion_init(t_kernel_config*);
void* atender_consola(void* cliente_fd);
void* atender_cpu_dispatch(void* arg);
void iterator(instruccion* );
u_int32_t siguiente_pid();
void dirigir_pcb(t_pcb* pcb);

#endif /* KERNEL_INCLUDE_KERNEL_H_ */
