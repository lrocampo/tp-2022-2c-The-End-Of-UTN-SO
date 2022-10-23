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

#define RUTA_LOGGER_CPU "./cpu.log"
#define RUTA_LOGGER_DEBUG_CPU "./cpu_db.log"
#define NOMBRE_MODULO "CPU"
#define RUTA_CPU_CONFIG "./src/cpu.config"

t_log *cpu_logger;
t_cpu_config* cpu_config;

void ejecutar(t_pcb*);

#endif /* CPU_INCLUDE_CPU_H_ */
