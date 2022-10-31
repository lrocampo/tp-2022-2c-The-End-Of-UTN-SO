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
t_kernel_config* kernel_config;

t_queue* cola_consolas;
t_queue* cola_new_pcbs;
t_queue* cola_exit_pcbs;
t_queue* cola_ready_FIFO_pcbs;
t_queue* cola_ready_RR_pcbs;

// t_queue* cola_ready_pcbs; // despues la borramos

sem_t consolas;
sem_t conexiones;
sem_t multiprogramacion;
sem_t procesos_finalizados; // existe alguna convencion para el nombre de los semaforos de sincro?
sem_t interrupcion_quantum;
uint32_t pid_actual;
pthread_mutex_t pid_mutex;
pthread_mutex_t cola_consolas_mutex;
// pthread_mutex_t cola_ready_pcbs_mutex;// desp vuela
pthread_mutex_t cola_ready_RR_pcbs_mutex;
pthread_mutex_t cola_ready_FIFO_pcbs_mutex;
pthread_t th_timer;
int kernel_server_fd;

void iniciar_conexiones_con_cpu();
void colas_init();
void semaforos_init(); 
void threads_init();
void planificacion_init();
//void* atender_consola(void* cliente_fd);
void* atender_cpu_dispatch(void*);
void* atender_cpu_interrupt(void*);
void iterator(instruccion* );
u_int32_t siguiente_pid();
void dirigir_pcb(t_pcb*);
void agregar_pcb_a_ready();
void* atender_consolas();
void esperar_conexiones();
void iniciar_interrupcion();
void* enviar_interrupt(void*);
void push_ready_pcb(t_pcb*);
t_pcb* pop_ready_pcb();
void ejecutar_espera(uint32_t);
void cambiar_estado(t_pcb*, estado_proceso);

#endif /* KERNEL_INCLUDE_KERNEL_H_ */
