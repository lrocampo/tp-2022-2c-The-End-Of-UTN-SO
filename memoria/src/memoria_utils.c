#include <memoria_utils.h>

t_log *memoria_logger;
t_memoria_config* memoria_config;
int memoria_server_cpu_fd;
int memoria_server_kernel_fd;
int cliente_kernel_fd;
int cliente_cpu_fd;
t_list* lista_de_marcos;
t_list* lista_de_marcos_swap;
t_list* lista_de_tablas_de_paginas;

void* espacio_memoria;

pthread_t th_atender_pedido_de_memoria;
pthread_t th_atender_kernel;

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
    pthread_join(th_atender_kernel, NULL);
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
	free(memoria_config->path_swap);
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
	lista_de_marcos = list_create();
	marcos_init(lista_de_marcos, memoria_config->tamanio_memoria, memoria_config->tamanio_pagina);
}

void marcos_init(t_list* lista_marcos, int tamanio_memoria, int tamanio_pagina){
	int cantidad_de_marcos = tamanio_memoria / tamanio_pagina;
	log_debug(memoria_logger, "Cargando %d marcos", cantidad_de_marcos);
	for(int i = 0; i < cantidad_de_marcos; i++) {
		t_marco *marco = malloc(sizeof(t_marco));
		marco->numero_marco = i;
		marco->pid = -1;
		list_add(lista_marcos, marco);
	}
	log_debug(memoria_logger, "%d marcos cargados correctamente", list_size(lista_marcos));
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
		t_tabla_de_paginas* tabla_de_paginas = malloc(sizeof(t_tabla_de_paginas));
		tabla_de_paginas->entradas = list_create();
		tabla_de_paginas->indice_tabla_de_pagina = i;
		tabla_de_paginas->pid = pcb->pid;
		for(j = 0; j < cantidad_de_paginas; j++) {
			t_entrada_tp* nueva_entrada_pagina = malloc(sizeof(t_entrada_tp));
			nueva_entrada_pagina->marco = -1; 
			nueva_entrada_pagina->presencia = false;
			nueva_entrada_pagina->uso = false;
			nueva_entrada_pagina->modificado = false;
			nueva_entrada_pagina->posicion_swap = obtener_posicion_libre_swap(); // obtener posicion libre swap
			ocupar_posicion_swap(pcb->pid, nueva_entrada_pagina->posicion_swap);
			list_add(tabla_de_paginas->entradas, nueva_entrada_pagina);
		}
		list_add(lista_de_tablas_de_paginas, tabla_de_paginas);
	}
}

/* Conexiones con Kernel y CPU */

void solicitudes_a_memoria_init() {
    pthread_create(&th_atender_pedido_de_memoria, NULL, &atender_pedido_de_memoria, NULL);

	pthread_create(&th_atender_kernel, NULL, &atender_kernel, NULL);
}

void* atender_pedido_de_memoria(void* args){
	memoria_server_cpu_fd = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_cpu);
	if(memoria_server_cpu_fd == -1){
		pthread_exit(NULL);
	}
	cliente_cpu_fd = esperar_cliente(memoria_server_cpu_fd);
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

void* atender_kernel(void* args) {
	memoria_server_kernel_fd = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_kernel);
	if(memoria_server_kernel_fd == -1){
		pthread_exit(NULL);
	}
 	cliente_kernel_fd = esperar_cliente(memoria_server_kernel_fd);
 	log_debug(memoria_logger,"Se conecto un Kernel a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_kernel_fd);
		switch (mensaje)
		{
		case ESTRUCTURAS:
			t_pcb_memoria* pcb = recibir_pcb_memoria(cliente_kernel_fd);
			log_debug(memoria_logger, "Recibi pcb con pid: %d",pcb->pid);
			crear_tablas_de_pagina(pcb);
			t_list* indices = obtener_indices_tablas_de_pagina(pcb);
			mensaje = OKI_ESTRUCTURAS;
			log_debug(memoria_logger, "llegue aca, por eviar cosas");
 			enviar_indices_tabla_paginas(indices, cliente_kernel_fd);
			list_destroy_and_destroy_elements(indices, free);
			pcb_memoria_destroy(pcb);
			break;
		case PAGINA:
			t_pagina* pagina = recibir_pagina(cliente_kernel_fd);
			log_debug(memoria_logger, "Messi.");
			// mutex
			t_tabla_de_paginas* tabla_paginas = list_get(lista_de_tablas_de_paginas, pagina->indice_tabla_de_pagina);
			// mutex
			int cantidad_paginas_proceso = cantidad_de_paginas_en_memoria_proceso(tabla_paginas->pid);
			if(cantidad_paginas_proceso < memoria_config->marcos_por_proceso){
				// obtener marco libre
			}
			else {
				// ejecutar algoritmo
			}

			mensaje = OKI_PAGINA;
 			enviar_datos(cliente_kernel_fd, &mensaje, sizeof(cod_mensaje));		
			break;
		default:
			log_debug(memoria_logger,"Se desconecto el cliente.");
			pthread_exit(NULL);
		}
 	}
 }

int cantidad_de_paginas_en_memoria_proceso(int pid){
	int cantidad = 0;

	t_list* tablas_paginas_proceso = obtener_tablas_por_pid(pid);
	for(int i = 0; i < list_size(tablas_paginas_proceso); i++){
		t_tabla_de_paginas* tabla = list_get(tablas_paginas_proceso, i);
		t_list* entradas_presentes = list_filter(tabla->entradas, (void*) pagina_presente);
		cantidad += list_size(entradas_presentes);
		list_destroy(entradas_presentes);
	}
	list_destroy(tablas_paginas_proceso);
	return cantidad;
}

bool pagina_presente(t_entrada_tp* entrada){
	return entrada->presencia;
}

t_list* obtener_indices_tablas_de_pagina(t_pcb_memoria* pcb){
	
	t_list* tablas_buscadas;
	t_list* indices;
	
	// bool by_pid(t_tabla_de_paginas* tabla){
	// 	return tabla->pid == pcb->pid;
	// }

	tablas_buscadas = obtener_tablas_por_pid(pcb->pid);

	char* to_idx(t_tabla_de_paginas* tabla){
		return string_itoa(tabla->indice_tabla_de_pagina);
	}	

	// tablas_buscadas = list_filter(lista_de_tablas_de_paginas, (void*) by_pid);

	log_debug(memoria_logger, "cantidad de tablas de pid: %d : %d", pcb->pid, list_size(tablas_buscadas));

	indices = list_map(tablas_buscadas, (void*) to_idx);
	list_destroy(tablas_buscadas);

	return indices;
}

t_list* obtener_tablas_por_pid(int pid){
	bool by_pid(t_tabla_de_paginas* tabla){
		return tabla->pid == pid;
	}
	return list_filter(lista_de_tablas_de_paginas, (void*) by_pid);
}

void escribir_en_memoria_principal(int direccion_fisica, int *valor) {
	pthread_mutex_lock(&memoria_usuario_mutex);
	memcpy(espacio_memoria + direccion_fisica, valor, sizeof(int));
	pthread_mutex_unlock(&memoria_usuario_mutex);
}

int leer_en_memoria_principal(int direccion_fisica) {
	int valor;
	pthread_mutex_lock(&memoria_usuario_mutex);
    memcpy(&valor, espacio_memoria + direccion_fisica, sizeof(int));
    pthread_mutex_unlock(&memoria_usuario_mutex);

	return valor;
 }

 t_entrada_tp* obtener_entrada_tp(t_pagina* pagina){
	pthread_mutex_lock(&lista_de_tablas_de_paginas_mutex);
	t_tabla_de_paginas* tabla_de_paginas = list_get(lista_de_tablas_de_paginas, pagina->indice_tabla_de_pagina);
	t_entrada_tp* entrada_pagina = list_get(tabla_de_paginas->entradas, pagina->numero_pagina);
	pthread_mutex_unlock(&lista_de_tablas_de_paginas_mutex);
	return entrada_pagina;
 }

 int obtener_numero_de_marco(t_pagina* pagina) {
	t_entrada_tp* entrada_pagina = obtener_entrada_tp(pagina);

	if(entrada_pagina->presencia == 0) {
		return PAGE_FAULT;
	}
	else {
		return entrada_pagina->marco;
	}
 }

 /* Utils */

 bool marco_libre(t_marco* marco){
    return marco->pid == -1;
}