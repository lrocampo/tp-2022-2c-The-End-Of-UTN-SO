/*
 * kernel.h
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#ifndef KERNEL_INCLUDE_KERNEL_H_
#define KERNEL_INCLUDE_KERNEL_H_

#include <kernel_utils.h>

#define RUTA_LOGGER_KERNEL "./kernel.log"
#define RUTA_LOGGER_DEBUG_KERNEL "./kernel_db.log"
#define NOMBRE_MODULO "Kernel"
#define RUTA_KERNEL_CONFIG "./src/kernel.config"

extern int conexion_cpu_dispatch;
extern int conexion_cpu_interrupt;
extern int cantidad_dispositivos;
extern int kernel_server_fd;

extern t_log* kernel_logger;
extern t_kernel_config* kernel_config;
extern t_list* dispositivos_io;

void esperar_conexiones();
void iniciar_conexiones_con_cpu();
// TODO: iniciar_conexion_con_memoria
void planificacion_init();

#endif /* KERNEL_INCLUDE_KERNEL_H_ */
