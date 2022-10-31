/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <kernel.h>

// CONSOLA_FD PUEDE SER PPID

int main(void){
	
	/* LOGGER DE ENTREGA */
	//kernel_logger = iniciar_logger(RUTA_LOGGER_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	
	/* LOGGER DE DEBUG */
	kernel_logger = iniciar_logger(RUTA_LOGGER_DEBUG_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(kernel_logger,"Arrancando kernel...");

	kernel_config = cargar_configuracion(RUTA_KERNEL_CONFIG, KERNEL);
	log_debug(kernel_logger,"Configuracion cargada correctamente");

	kernel_server_fd = iniciar_servidor(kernel_config->ip_kernel, kernel_config->puerto_escucha);

	iniciar_conexiones_con_cpu();

	planificacion_init();

	esperar_conexiones();
	
	log_debug(kernel_logger,"Termino Kernel");

	return EXIT_SUCCESS;
}

void iniciar_conexiones_con_cpu() {
	conexion_cpu_dispatch = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch);
	if(conexion_cpu_dispatch != -1){
		log_debug(kernel_logger,"Conexion creada correctamente con CPU DISPATCH");
	}

	conexion_cpu_interrupt = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt);
		if(conexion_cpu_interrupt != -1){
		log_debug(kernel_logger,"Conexion creada correctamente con CPU INTERRUPT");
	}
}

void esperar_conexiones(){
    pthread_join(th_conexiones, NULL);
}

void planificacion_init(/*t_kernel_config* kernel_config*/) {
	algoritmo = kernel_config->algoritmo;
	pid_actual = 0;
	pthread_mutex_init(&pid_mutex, NULL);
	/* Al encender el kernel, arrancamos con pid 0 */
    sem_init(&multiprogramacion,0,kernel_config->grado_multiprogramacion);

	largo_plazo_init();
    corto_plazo_init();
}
