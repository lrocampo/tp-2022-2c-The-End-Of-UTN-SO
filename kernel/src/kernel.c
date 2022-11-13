/*
 * kernel.c
 *
 *  Created on: Sep 11, 2022
 *      Author: utnso
 */

#include <kernel.h>

void sighandler(int s){
    terminar_modulo();
    exit(0);
}

int main(void){

	signal(SIGINT, sighandler);

	/* LOGGER DE ENTREGA */
	//kernel_logger = iniciar_logger(RUTA_LOGGER_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_INFO);
	
	/* LOGGER DE DEBUG */
	kernel_logger = iniciar_logger(RUTA_LOGGER_DEBUG_KERNEL, NOMBRE_MODULO, 1, LOG_LEVEL_DEBUG);

	log_debug(kernel_logger,"Arrancando kernel...");

	kernel_config = cargar_configuracion(RUTA_KERNEL_CONFIG, configurar_kernel);
	log_debug(kernel_logger,"Configuracion cargada correctamente");

	kernel_server_fd = iniciar_servidor(kernel_config->ip_kernel, kernel_config->puerto_escucha);

	iniciar_conexiones_con_cpu();

	iniciar_conexion_con_memoria();

	planificacion_init();

	esperar_conexiones();

	log_debug(kernel_logger,"Termino Kernel");

	terminar_modulo();
	
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

void iniciar_conexion_con_memoria() {
 	conexion_memoria = crear_conexion(kernel_config->ip_memoria, kernel_config->puerto_memoria);
	if(conexion_memoria != -1){
 		log_debug(kernel_logger, "Conexion creada correctamente con MEMORIAs");
	}
}

void esperar_conexiones(){
    pthread_join(th_conexiones, NULL);
}

void planificacion_init(/*t_kernel_config* kernel_config*/) {
	algoritmo = kernel_config->algoritmo;
	dispositivos_io = kernel_config->dispositivos_io;
	cantidad_dispositivos = list_size(dispositivos_io);
	pid_actual = 0;
	pthread_mutex_init(&pid_mutex, NULL);
	/* Al encender el kernel, arrancamos con pid 0 */
    sem_init(&multiprogramacion,0,kernel_config->grado_multiprogramacion);

	largo_plazo_init();
    corto_plazo_init();
	dispositivos_io_init();
}

void * configurar_kernel(t_config* config){
	t_kernel_config* kernel_config;
	kernel_config = malloc(sizeof(t_kernel_config));
	kernel_config->ip_cpu = strdup(config_get_string_value(config, "IP_CPU"));
	kernel_config->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
	kernel_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
	kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config, "PUERTO_CPU_DISPATCH"));
	kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config, "PUERTO_CPU_INTERRUPT"));
	kernel_config->grado_multiprogramacion = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	kernel_config->algoritmo = config_get_algoritmo_enum(config);
	kernel_config->quantum_RR = config_get_int_value(config, "QUANTUM_RR");
	kernel_config->ip_memoria = strdup(config_get_string_value(config,"IP_MEMORIA"));
	kernel_config->puerto_memoria = strdup(config_get_string_value(config,"PUERTO_MEMORIA"));
	kernel_config->dispositivos_io = config_get_io_list(config); 
	return kernel_config;
}

void terminar_modulo(){
	liberar_conexion(conexion_cpu_dispatch);
	liberar_conexion(conexion_cpu_interrupt);
	liberar_conexion(conexion_memoria);
	pthread_cancel(th_ejecucion);
	pthread_cancel(th_rajar_pcb);
	pthread_cancel(th_transiciones_ready);
	log_destroy(kernel_logger);
	kernel_config_destroy();
}

void kernel_config_destroy(){
	free(kernel_config->puerto_memoria);
	free(kernel_config->puerto_cpu_dispatch);
	free(kernel_config->puerto_cpu_interrupt);
	free(kernel_config->puerto_escucha);
	free(kernel_config->ip_cpu);
	free(kernel_config->ip_kernel);
	free(kernel_config->ip_memoria);
	list_destroy_and_destroy_elements(kernel_config->dispositivos_io,dispositivo_io_destroy);
	free(kernel_config);
}

void dispositivo_io_destroy(void* arg){
	t_dispositivo* dispositivo_io = (t_dispositivo*) arg;
	free(dispositivo_io->nombre);
	queue_destroy_and_destroy_elements(dispositivo_io->cola,pcb_destroy);
	free(dispositivo_io);
}