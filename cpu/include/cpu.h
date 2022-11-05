/*
 * cpu.h
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#ifndef CPU_INCLUDE_CPU_H_
#define CPU_INCLUDE_CPU_H_

#include <cpu_utils.h>

extern t_log *cpu_logger;
extern t_cpu_config* cpu_config;
extern int server_fd_dispatch;
extern int cliente_fd_dispatch;
extern int conexion_memoria;
extern bool interrupcion;

extern pthread_t th_kernel_dispatch;
extern pthread_t th_kernel_interrupt;

extern pthread_mutex_t interrupcion_mutex;

#endif /* CPU_INCLUDE_CPU_H_ */
