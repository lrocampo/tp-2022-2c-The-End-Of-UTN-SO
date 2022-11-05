#include <memoria_utils.h>

t_log *memoria_logger;
t_memoria_config* memoria_config;
int server_fd_cpu;
int server_fd_kernel;
int cliente_kernel_fd;
int cliente_cpu_fd;

pthread_t th_atender_pedido_de_memoria;
pthread_t th_atender_pedido_de_estructuras;


/* Configuracion y limpieza */
void * configurar_memoria(t_config* config){
	t_memoria_config* memoria_config;
	memoria_config = malloc(sizeof(t_memoria_config));
	memoria_config->ip_memoria =  strdup(config_get_string_value(config, "IP_MEMORIA"));
	memoria_config->ip_cpu =  strdup(config_get_string_value(config, "IP_CPU"));
	memoria_config->ip_kernel =  strdup(config_get_string_value(config, "IP_KERNEL"));
	memoria_config->puerto_escucha_cpu = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_CPU"));
	memoria_config->puerto_escucha_kernel = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_KERNEL"));
	return memoria_config;
}

void esperar_conexiones() {
    pthread_join(th_atender_pedido_de_memoria, NULL);
    pthread_join(th_atender_pedido_de_estructuras, NULL);
}

void terminar_modulo(){
	log_destroy(memoria_logger);
	memoria_config_destroy();
}

void memoria_config_destroy(){
	free(memoria_config->ip_cpu);
	free(memoria_config->ip_kernel);
	free(memoria_config->ip_memoria);
	free(memoria_config->puerto_escucha_cpu);
	free(memoria_config->puerto_escucha_kernel);
	free(memoria_config);
}

/* Conexiones con Kernel y CPU */

void solicitudes_a_memoria_init() {
    pthread_create(&th_atender_pedido_de_memoria, NULL, &atender_pedido_de_memoria, NULL);

	pthread_create(&th_atender_pedido_de_estructuras, NULL, &atender_pedido_de_estructuras, NULL);
}

void* atender_pedido_de_memoria(void* args){
	server_fd_cpu = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_cpu);
	if(server_fd_cpu == -1){
		pthread_exit(NULL);
	}
	cliente_cpu_fd = esperar_cliente(server_fd_cpu);
	log_debug(memoria_logger,"Se conecto un CPU a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_cpu_fd);
		if(mensaje == MENSAJE){
			recibir_mensaje(memoria_logger, cliente_cpu_fd);
			enviar_mensaje("Quien te conoce pa?", cliente_cpu_fd);
		}
		else {
			log_debug(memoria_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
		}
	}
	
}

void* atender_pedido_de_estructuras(void* args) {
	server_fd_kernel = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_kernel);
	if(server_fd_kernel == -1){
		pthread_exit(NULL);
	}
 	cliente_kernel_fd = esperar_cliente(server_fd_kernel);
 	log_debug(memoria_logger,"Se conecto un Kernel a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_kernel_fd);
 		if(mensaje == MENSAJE){
 			recibir_mensaje(memoria_logger, cliente_kernel_fd);
 			enviar_mensaje("Atiendo boludos", cliente_kernel_fd);
 		}
 		else {
 			log_debug(memoria_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
 		}
 	}
 }