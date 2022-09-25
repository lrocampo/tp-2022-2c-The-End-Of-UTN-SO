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
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#define RUTA_LOGGER_KERNEL "/home/utnso/tp-2022-2c-The-End-Of-UTN-SO/kernel/kernel.log"
#define NOMBRE_MODULO "Kernel"
#define RUTA_KERNEL_CONFIG "./src/kernel.config"

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;

//t_socket* socketEscucha;
t_log *kernel_logger;

int inicializar();

#endif /* KERNEL_INCLUDE_KERNEL_H_ */