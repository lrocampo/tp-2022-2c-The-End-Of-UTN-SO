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

void esperar_conexiones();
void iniciar_conexiones_con_cpu();
void iniciar_conexion_con_memoria();
void planificacion_init();
void * configurar_kernel(t_config*);
void dispositivo_io_destroy(void*);
void kernel_config_destroy();
void terminar_modulo();

#endif /* KERNEL_INCLUDE_KERNEL_H_ */
