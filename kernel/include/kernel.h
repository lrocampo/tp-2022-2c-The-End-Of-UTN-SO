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
#define NOMBRE_MODULO "Kernel"
#define RUTA_KERNEL_CONFIG "./src/kernel.config"

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;

//t_socket* socketEscucha;
t_log *kernel_logger;

t_queue* pcbs;
sem_t consolas;
sem_t multiprogramacion;
uint32_t pid_actual;
pthread_mutex_t pid_mutex;


int inicializar();
void* atender_consola(void* cliente_fd);
void* atender_cpu_dispatch(void* arg);

#endif /* KERNEL_INCLUDE_KERNEL_H_ */