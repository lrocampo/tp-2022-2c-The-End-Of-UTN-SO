#include <memoria_utils.h>

t_log *memoria_logger;
t_memoria_config* memoria_config;
int server_fd_cpu;
int server_fd_kernel;
int cliente_kernel_fd;
int cliente_cpu_fd;
t_list* lista_de_marcos;
t_list* lista_de_marcos_swap;
t_list* lista_de_tablas_de_paginas;

void* espacio_memoria;

pthread_t th_atender_pedido_de_memoria;
pthread_t th_atender_pedido_de_estructuras;

pthread_mutex_t memoria_swap_mutex;
pthread_mutex_t memoria_usuario_mutex;
pthread_mutex_t lista_de_tablas_de_paginas_mutex;
pthread_mutex_t lista_de_tablas_de_paginas_swap_mutex;

void* swap;

/* Configuracion y limpieza */
void * configurar_memoria(t_config* config){
	t_memoria_config* memoria_config;
	memoria_config = malloc(sizeof(t_memoria_config));
	memoria_config->ip_memoria =  strdup(config_get_string_value(config, "IP_MEMORIA"));
	memoria_config->ip_cpu =  strdup(config_get_string_value(config, "IP_CPU"));
	memoria_config->ip_kernel =  strdup(config_get_string_value(config, "IP_KERNEL"));
	memoria_config->puerto_escucha_cpu = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_CPU"));
	memoria_config->puerto_escucha_kernel = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_KERNEL"));
	memoria_config->tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
	memoria_config->tamanio_pagina = config_get_int_value(config, "TAM_PAGINA");
	memoria_config->tamanio_swap = config_get_int_value(config, "TAMANIO_SWAP");
	memoria_config->retardo_swap = config_get_int_value(config, "RETARDO_SWAP");
	memoria_config->entradas_por_tabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
	memoria_config->path_swap =  strdup(config_get_string_value(config, "PATH_SWAP"));
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

/* Inicializacion */

void memoria_principal_init() {
	pthread_mutex_init(&memoria_usuario_mutex, NULL);
	log_debug(memoria_logger, "Creando espacio de memoria...");
	espacio_memoria = malloc(memoria_config->tamanio_memoria);

	marcos_memoria_principal_init();

	lista_de_tablas_de_paginas = list_create();
}

void marcos_memoria_principal_init() {
	log_debug(memoria_logger, "Cargando marcos memoria principal...");
	marcos_init(lista_de_marcos, memoria_config->tamanio_memoria, memoria_config->tamanio_pagina);
}

void marcos_init(t_list* lista_marcos, int tamanio_memoria, int tamanio_pagina){
	lista_marcos = list_create();
	int cantidad_de_marcos = tamanio_memoria / tamanio_pagina;

	for(int i = 0; i < cantidad_de_marcos; i++) {
		t_marco *marco = (t_marco*)malloc(sizeof(t_marco));
		marco->numero_marco = i;
		marco->pid = -1;
		list_add(lista_marcos, marco);
	}
}

void algoritmos_init() {
	log_debug(memoria_logger, "Obteniendo algoritmo...");
}

void crear_tablas_de_pagina(t_pcb_memoria* pcb) {
	int i = 0;
	int j = 0;
	int cantidad_de_segmentos = list_size(pcb->segmentos);
	int cantidad_de_paginas = memoria_config->entradas_por_tabla;
	for(i = 0; i < cantidad_de_segmentos; i++) {
		for(j = 0; j < cantidad_de_paginas; j++) {
			t_pagina* nueva_pagina = malloc(sizeof(t_pagina));
			nueva_pagina->marco = j;
			nueva_pagina->indice_tabla_de_pagina = i;
			nueva_pagina->presencia = false;
			nueva_pagina->uso = false;
			nueva_pagina->modificado = false;
			nueva_pagina->posicion_swap = 1; // no se que poner aca
		}
	}
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
 		if(mensaje == ESTRUCTURAS){
			t_pcb_memoria* pcb = recibir_pcb_memoria(cliente_kernel_fd);
			log_debug(memoria_logger, "Recibi pcb con pid: %d",pcb->pid);
			crear_tablas_de_pagina(pcb);
			// esto esta mal, porque deberia enviar la tabla de segmentos
			cod_mensaje cod_msj = OKI_ESTRUCTURAS;
 			enviar_datos(cliente_kernel_fd, &cod_msj, sizeof(cod_msj));
 		}
 		else {
 			log_debug(memoria_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
 		}
 	}
 }

void escribir_en_memoria_principal(int direccion_fisica, int valor) {
	pthread_mutex_lock(&memoria_usuario_mutex);
	memcpy(espacio_memoria + direccion_fisica, valor, sizeof(int));
	pthread_mutex_unlock(&memoria_usuario_mutex);
}

int leer_en_memoria_principal(int direccion_fisica) {
	int valor;
	pthread_mutex_lock(&memoria_usuario_mutex);
    memcpy(valor, espacio_memoria + direccion_fisica, sizeof(int));
    pthread_mutex_unlock(&memoria_usuario_mutex);

	return valor;
 }

 int obtener_numero_de_marco(t_pagina* pagina, int numero_pagina) {
	pthread_mutex_lock(&lista_de_tablas_de_paginas_mutex);
	t_list* lista = list_get(lista_de_tablas_de_paginas, pagina->indice_tabla_de_pagina);
	t_pagina* pagina = list_get(lista, numero_pagina);
	pthread_mutex_unlock(&lista_de_tablas_de_paginas_mutex);

	if(pagina->presencia == 0) {
		return PAGE_FAULT;
	}
	else {
		return pagina->marco;
	}
 }

 /* Utils */

 bool marco_libre(void* marco){
    return ((t_marco*) marco)->pid == -1;
}