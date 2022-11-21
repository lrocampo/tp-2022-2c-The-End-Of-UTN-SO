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
void* swap;

pthread_t th_atender_cpu;
pthread_t th_atender_kernel;

t_list* cursores;

pthread_mutex_t memoria_swap_mutex;
pthread_mutex_t memoria_usuario_mutex;
pthread_mutex_t lista_de_tablas_de_paginas_mutex;
pthread_mutex_t lista_de_tablas_de_paginas_swap_mutex;


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
	memoria_config->retardo_memoria =  config_get_int_value(config, "RETARDO_MEMORIA");
	memoria_config->marcos_por_proceso =  config_get_int_value(config, "MARCOS_POR_PROCESO");
	memoria_config->algoritmo_reemplazo = config_get_algoritmo_enum(config, "ALGORITMO_REEMPLAZO");
	return memoria_config;
}

void esperar_conexiones() {
    pthread_join(th_atender_cpu, NULL);
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
	cursores = list_create();
	pthread_mutex_init(&memoria_usuario_mutex, NULL);
	pthread_mutex_init(&lista_de_tablas_de_paginas_mutex, NULL);

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

/* Conexiones con Kernel y CPU */

void solicitudes_a_memoria_init() {
    pthread_create(&th_atender_cpu, NULL, &atender_cpu, NULL);

	pthread_create(&th_atender_kernel, NULL, &atender_kernel, NULL);
}

void atender_pedido_de_marco() {
	cod_mensaje mensaje;
	log_debug(memoria_logger, "CPU - Recibiendo pedido de marco");
	t_pagina* pagina = recibir_pagina(cliente_cpu_fd);
	log_debug(memoria_logger, "CPU - Pagina solicitada: %d", pagina->numero_pagina);
	int marco = obtener_numero_de_marco(pagina);
	if(marco == PAGE_FAULT) {
		mensaje = PAGE_NOT_FOUND_404;
		enviar_datos(cliente_cpu_fd, &mensaje, sizeof(mensaje));
	}
	else {
		mensaje = OKI_MARCO;
		enviar_valor_con_codigo(marco, mensaje, cliente_cpu_fd);
	}
	free(pagina);
}

void atender_pedido_de_lectura() {
	int direccion_fisica;
	int valor;
	cod_mensaje mensaje;

	log_debug(memoria_logger, "CPU - Recibiendo pedido de lectura");
	ejecutar_espera(memoria_config->retardo_memoria);
	direccion_fisica = recibir_valor(cliente_cpu_fd);
	valor = leer_en_memoria_principal(direccion_fisica);
	mensaje = OKI_LEER;
	log_debug(memoria_logger, "CPU - dir fisica: %d, valor leido: %d", direccion_fisica, valor);
	enviar_valor_con_codigo(valor, mensaje, cliente_cpu_fd);
}

void atender_pedido_de_escritura() {
	int direccion_fisica;
	int valor;
	cod_mensaje mensaje;

	log_debug(memoria_logger, "CPU - Recibiendo pedido de escritura");
	ejecutar_espera(memoria_config->retardo_memoria);
	recibir_datos(cliente_cpu_fd, &direccion_fisica, sizeof(int));
	recibir_datos(cliente_cpu_fd, &valor, sizeof(int));
	log_debug(memoria_logger, "CPU - dir fisica: %d, valor a escribir: %d", direccion_fisica, valor);
	escribir_en_memoria_principal(direccion_fisica, valor);
	mensaje = OKI_ESCRIBIR;
	enviar_datos(cliente_cpu_fd, &mensaje, sizeof(mensaje));
}

void* atender_cpu(void* args){
	memoria_server_cpu_fd = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_cpu);
	if(memoria_server_cpu_fd == -1){
		pthread_exit(NULL);
	}
	cliente_cpu_fd = esperar_cliente(memoria_server_cpu_fd);
	log_debug(memoria_logger,"CPU se conecto a MEMORIA.");

	enviar_configuracion_memoria(memoria_config->tamanio_pagina, memoria_config->entradas_por_tabla, cliente_cpu_fd);

	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_cpu_fd);
		
		switch (mensaje)
		{
		case PAGINA:
			atender_pedido_de_marco();
			break;
		case LEER:
			atender_pedido_de_lectura();
			break;
		case ESCRIBIR:
			atender_pedido_de_escritura();
			break;
		default:
			log_debug(memoria_logger,"CPU se desconecto.");
			pthread_exit(NULL);
			break;
		}
	}
	
}

void* atender_kernel(void* args) {
	memoria_server_kernel_fd = iniciar_servidor(memoria_config->ip_memoria, memoria_config->puerto_escucha_kernel);
	if(memoria_server_kernel_fd == -1){
		pthread_exit(NULL);
	}
 	cliente_kernel_fd = esperar_cliente(memoria_server_kernel_fd);
 	log_debug(memoria_logger,"KERNEL se conecto a MEMORIA.");
	while(1){
		cod_mensaje mensaje = recibir_operacion(cliente_kernel_fd);
		switch (mensaje)
		{
		case ESTRUCTURAS:
			atender_pedido_de_estructuras();
			break;
		case LIBERAR_ESTRUCTURAS:
			atender_liberar_estructuras();
			break;
		case PAGINA:
			atender_pedido_de_pagina_fault();		
			break;
		default:
			log_debug(memoria_logger,"KERNEL se desconecto.");
			pthread_exit(NULL);
		}
 	}
 }

void atender_pedido_de_estructuras(){
	t_pcb_memoria* pcb = recibir_pcb_memoria(cliente_kernel_fd);
	log_debug(memoria_logger, "KERNEL- Recibi pcb con pid: %d",pcb->pid);
	crear_tablas_de_pagina(pcb);
	t_list* indices = obtener_indices_tablas_de_pagina(pcb);
	enviar_indices_tabla_paginas(indices, cliente_kernel_fd);
	list_destroy_and_destroy_elements(indices, free);
	pcb_memoria_destroy(pcb);
}

void atender_pedido_de_pagina_fault(){
	t_pagina* pagina = recibir_pagina(cliente_kernel_fd);
	t_marco* marco_libre;
	log_debug(memoria_logger, "pagina page fault: %d", pagina->numero_pagina);
	pthread_mutex_lock(&lista_de_tablas_de_paginas_mutex);
	t_tabla_de_paginas* tabla_paginas = list_get(lista_de_tablas_de_paginas, pagina->indice_tabla_de_pagina);
	pthread_mutex_unlock(&lista_de_tablas_de_paginas_mutex);
	log_debug(memoria_logger, "Tabla de paginas: %d", pagina->indice_tabla_de_pagina);
	int cantidad_marcos_cargados = cantidad_de_marcos_en_memoria_proceso(tabla_paginas->pid);
	log_debug(memoria_logger, "Cantidad de marcos en memoria del proceso: %d", cantidad_marcos_cargados);
	if(cantidad_marcos_cargados == memoria_config->marcos_por_proceso){
		t_entrada_tp* entrada_a_reemplazar = obtener_victima_a_reemplazar(tabla_paginas->pid);
		rajar_pagina(entrada_a_reemplazar);				
		marco_libre = list_get(lista_de_marcos, entrada_a_reemplazar->marco);
	}
	else{
		marco_libre = obtener_marco_libre(lista_de_marcos);
	}
	cargar_pagina_en_memoria_principal(pagina, marco_libre, tabla_paginas->pid);
	cod_mensaje mensaje = OKI_PAGINA;
	enviar_datos(cliente_kernel_fd, &mensaje, sizeof(cod_mensaje));
	free(pagina);
}

void atender_liberar_estructuras(){
	int pid = recibir_valor(cliente_kernel_fd);
	log_debug(memoria_logger, "KERNEL - Recibiendo pedido de liberar estructuras PID: %d",pid);
	liberar_memoria_de_proceso(pid);
	cod_mensaje mensaje = OKI_LIBERAR_ESTRUCTURAS;
	enviar_datos(cliente_kernel_fd, &mensaje, sizeof(mensaje));
}

void escribir_en_memoria_principal(int direccion_fisica, int valor) {
	pthread_mutex_lock(&memoria_usuario_mutex);
	memcpy(espacio_memoria + direccion_fisica, &valor, sizeof(int));
	pthread_mutex_unlock(&memoria_usuario_mutex);
}

int leer_en_memoria_principal(int direccion_fisica) {
	int valor;
	pthread_mutex_lock(&memoria_usuario_mutex);
    memcpy(&valor, espacio_memoria + direccion_fisica, sizeof(int));
    pthread_mutex_unlock(&memoria_usuario_mutex);
	return valor;
 }

 void liberar_memoria_de_proceso(int pid) {
	log_debug(memoria_logger, "Liberando memoria del proceso %d", pid);

	t_list* tablas = obtener_tablas_por_pid(pid);
	int i, j;

	for(i = 0; i < list_size(tablas); i++) {
		t_tabla_de_paginas* tabla = list_get(tablas, i); 
		for(j = 0; j < list_size(tabla->entradas); j++) {
			t_entrada_tp* entrada = list_get(tabla->entradas, j);
			liberar_marco_memoria(entrada);
			liberar_marco_swap(entrada);
		}
	}
	list_destroy(tablas);
	log_debug(memoria_logger, "Memoria liberada con exito PID: %d. Vuelvas prontos.", pid);
 }

 void liberar_marco_memoria(t_entrada_tp* entrada){
	if(entrada->presencia) {
		t_marco* marco = list_get(lista_de_marcos, entrada->marco);
		marco->pid = -1;
	}
 }

 void liberar_marco_swap(t_entrada_tp* entrada){
	t_marco* marco_swap = list_get(lista_de_marcos_swap, entrada->posicion_swap / memoria_config->tamanio_pagina);
	marco_swap->pid = -1;
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

	if(entrada_pagina->presencia) {
		log_debug(memoria_logger, "PRESENCIA de marco: %d", entrada_pagina->marco);
		return entrada_pagina->marco;
	}
	else {
		log_debug(memoria_logger, "PAGE FAULT: %d", pagina->numero_pagina);
		// actualizar bit de uso a 1?
		return PAGE_FAULT;
	}
 }

t_marco* obtener_marco_libre(t_list* marcos){
	return list_find(marcos,  (void*) marco_libre);
}

 /* Utils */

 bool marco_libre(t_marco* marco){
    return marco->pid == -1;
}