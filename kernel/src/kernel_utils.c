#include <kernel_utils.h>

int conexion_memoria;
int conexion_cpu_dispatch;
int conexion_cpu_interrupt;
int kernel_server_fd;
int cantidad_dispositivos;

t_log* kernel_logger;
t_kernel_config* kernel_config;
t_list* dispositivos_io;


t_queue* cola_new_pcbs;
t_queue* cola_exit_pcbs;
t_queue* cola_ready_FIFO_pcbs;
t_queue* cola_ready_RR_pcbs;

sem_t procesos_ready;
sem_t procesos_new;
sem_t multiprogramacion;
sem_t proceso_page_fault;
sem_t procesos_finalizados; 
sem_t interrupcion_quantum;
sem_t* s_dispositivos_io;
// chequear como inicializar array de semaforos

int pid_actual;
t_algoritmo algoritmo;

pthread_mutex_t pid_mutex;
pthread_mutex_t cola_new_pcbs_mutex;
pthread_mutex_t cola_ready_RR_pcbs_mutex;
pthread_mutex_t cola_ready_FIFO_pcbs_mutex;
pthread_mutex_t cola_exit_pcbs_mutex;
pthread_mutex_t* cola_dispositivo_mutex;

pthread_t th_timer;
pthread_t th_conexiones;
pthread_t th_ejecucion;
pthread_t th_transiciones_ready;
pthread_t th_manejo_page_fault;
pthread_t th_rajar_pcb;

/* Configuracion */

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

void * configurar_kernel(t_config* config){
	t_kernel_config* kernel_config;
	kernel_config = malloc(sizeof(t_kernel_config));
	kernel_config->ip_cpu = strdup(config_get_string_value(config, "IP_CPU"));
	kernel_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
	kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config, "PUERTO_CPU_DISPATCH"));
	kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config, "PUERTO_CPU_INTERRUPT"));
	kernel_config->grado_multiprogramacion = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	kernel_config->algoritmo = config_get_algoritmo_enum(config, "ALGORITMO_PLANIFICACION");
	kernel_config->quantum_RR = config_get_int_value(config, "QUANTUM_RR");
	kernel_config->ip_memoria = strdup(config_get_string_value(config,"IP_MEMORIA"));
	kernel_config->puerto_memoria = strdup(config_get_string_value(config,"PUERTO_MEMORIA"));
	kernel_config->dispositivos_io = config_get_io_list(config); 
	return kernel_config;
}

t_list* config_get_io_list(t_config* config){
	t_list* lista_dispositivos = list_create();
	char** array_dispositivos = config_get_array_value(config, "DISPOSITIVOS_IO");
	char** array_duraciones = config_get_array_value(config, "TIEMPOS_IO");
	for(int i = 0; i < string_array_size(array_dispositivos); i++){
		t_dispositivo* new_dispositivo = dispositivo_io_create(i, array_dispositivos, array_duraciones);
		list_add(lista_dispositivos, new_dispositivo);
	}
	string_array_destroy(array_dispositivos);
	string_array_destroy(array_duraciones);
	return lista_dispositivos;
}
// todo: liberar colas, semaforos y dispositivos io
void terminar_modulo(){
	liberar_conexion(conexion_cpu_dispatch);
	liberar_conexion(conexion_cpu_interrupt);
	liberar_conexion(conexion_memoria);
	log_destroy(kernel_logger);
	kernel_config_destroy();
}

void kernel_config_destroy(){
	free(kernel_config->puerto_memoria);
	free(kernel_config->puerto_cpu_dispatch);
	free(kernel_config->puerto_cpu_interrupt);
	free(kernel_config->puerto_escucha);
	free(kernel_config->ip_cpu);
	free(kernel_config->ip_memoria);
	list_destroy_and_destroy_elements(kernel_config->dispositivos_io,dispositivo_io_destroy);
	free(kernel_config);
}

/* Planificacion Utils */

void cambiar_estado(t_pcb* pcb, estado_proceso nuevo_estado){
	char* nuevo_estado_string = strdup(estado_to_string(nuevo_estado));
	char* estado_anterior_string = strdup(estado_to_string(pcb->estado));
	log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_anterior_string, nuevo_estado_string);
	pcb->estado = nuevo_estado;
	free(estado_anterior_string);
	free(nuevo_estado_string);
}

void log_cola_ready(){
	char* algoritmo_s = algoritmo_to_string(algoritmo);
	log_debug(kernel_logger, "Intentando loguear");

	pthread_mutex_lock(&cola_ready_RR_pcbs_mutex);
	pthread_mutex_lock(&cola_ready_FIFO_pcbs_mutex);
	if(algoritmo == FEEDBACK || algoritmo == RR){
		log_por_algoritmo(algoritmo_s, cola_ready_RR_pcbs);
	}
	if(algoritmo == FEEDBACK || algoritmo == FIFO){
		log_por_algoritmo(algoritmo_s, cola_ready_FIFO_pcbs);
	}
	pthread_mutex_unlock(&cola_ready_RR_pcbs_mutex);
	pthread_mutex_unlock(&cola_ready_FIFO_pcbs_mutex);
	
	//free(algoritmo_s);
}

void log_por_algoritmo(char* algoritmo_s, t_queue* cola){
	t_list* lista_a_loguear = pcb_queue_to_pid_list(cola);
	char* lista = list_to_string(lista_a_loguear);
	log_info(kernel_logger, "Cola Ready %s: [%s]",algoritmo_s, lista);
	list_destroy(lista_a_loguear);
	free(lista);
}

u_int32_t siguiente_pid(){
	u_int32_t siguiente_pid = 0;
	pthread_mutex_lock(&pid_mutex);
	siguiente_pid = ++pid_actual;
	pthread_mutex_unlock(&pid_mutex);
	return siguiente_pid;
}

t_pcb* pop_ready_pcb(){
	t_pcb* pcb;
	pthread_mutex_lock(&cola_ready_RR_pcbs_mutex);
	bool empty = queue_is_empty(cola_ready_RR_pcbs);
	pthread_mutex_unlock(&cola_ready_RR_pcbs_mutex);
	if((algoritmo == FEEDBACK && !empty) || algoritmo == RR){
        pcb = safe_pcb_pop(cola_ready_RR_pcbs, &cola_ready_RR_pcbs_mutex);
	} else {
		pcb = safe_pcb_pop(cola_ready_FIFO_pcbs, &cola_ready_FIFO_pcbs_mutex);
	}
	return pcb;
}

void push_ready_pcb(t_pcb* pcb){
	if((algoritmo == FEEDBACK && (pcb->estado == NEW || pcb->estado == BLOCK)) || algoritmo == RR) {
		pcb->con_desalojo = true;
		// saque un pcb->interrupcion = true; TODO: VALIDAR con las pruebas si esta bien
        safe_pcb_push(cola_ready_RR_pcbs, pcb, &cola_ready_RR_pcbs_mutex);
	} else {
		pcb->con_desalojo = false;
        safe_pcb_push(cola_ready_FIFO_pcbs, pcb, &cola_ready_FIFO_pcbs_mutex);
	}
		pcb->interrupcion = false;
		pcb->page_fault = false;
		pcb->segmentation_fault = false;
}

/* Utils */

instruccion* obtener_ultima_instruccion(t_pcb* pcb){
	int ultima_instruccion_idx = pcb->program_counter - 1;
	instruccion* ultima_instruccion = list_get(pcb->instrucciones,ultima_instruccion_idx);
	return ultima_instruccion;
}

void safe_pcb_push(t_queue* queue, t_pcb* pcb, pthread_mutex_t* mutex){
    pthread_mutex_lock(mutex);
    queue_push(queue, pcb);
    pthread_mutex_unlock(mutex);
}

t_pcb* safe_pcb_pop(t_queue* queue, pthread_mutex_t* mutex){
    t_pcb* pcb;
    pthread_mutex_lock(mutex);
    pcb = queue_pop(queue);
    pthread_mutex_unlock(mutex);
    return pcb;
}

