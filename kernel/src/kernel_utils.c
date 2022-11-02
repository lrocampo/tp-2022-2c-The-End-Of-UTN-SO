#include <kernel_utils.h>

int conexion_cpu_dispatch;
int conexion_cpu_interrupt;

t_log* kernel_logger;
t_kernel_config* kernel_config;
int kernel_server_fd;

t_queue* cola_new_pcbs;
t_queue* cola_exit_pcbs;
t_queue* cola_ready_FIFO_pcbs;
t_queue* cola_ready_RR_pcbs;

sem_t procesos_ready;
sem_t procesos_new;
sem_t multiprogramacion;
sem_t procesos_finalizados; 
sem_t interrupcion_quantum;

uint32_t pid_actual;
t_algoritmo algoritmo;

pthread_mutex_t pid_mutex;
pthread_mutex_t cola_new_pcbs_mutex;
pthread_mutex_t cola_ready_RR_pcbs_mutex;
pthread_mutex_t cola_ready_FIFO_pcbs_mutex;

pthread_t th_timer;
pthread_t th_conexiones;

/* Planificacion */

/* Largo Plazo */

void largo_plazo_init(){
    cola_new_pcbs = queue_create();
	cola_exit_pcbs = queue_create();
    pthread_t th_rajar_pcb;

	sem_init(&procesos_new,0,0);
	sem_init(&procesos_finalizados, 0, 0);
	pthread_mutex_init(&cola_new_pcbs_mutex, NULL);

    pthread_create(&th_rajar_pcb, NULL, &rajar_pcb, NULL);
	pthread_create(&th_conexiones, NULL, &atender_nueva_consola, NULL);
	pthread_detach(th_rajar_pcb);	
}

void* atender_nueva_consola(void* arg){
    t_pcb* pcb;
	t_list* instrucciones;
	while(1){
		log_debug(kernel_logger, "Soy Kernel. Esperando conexion...");
		int consola_fd = esperar_cliente(kernel_server_fd);
		log_debug(kernel_logger, "se conecto un cliente");
        cod_mensaje cod_msj = recibir_operacion(consola_fd);
        if(cod_msj == PAQUETE_INSTRUCCIONES){
            instrucciones = recibir_paquete_con_funcion(consola_fd, deserializar_instruccion);
            log_debug(kernel_logger, "Recibí %d instrucciones", list_size(instrucciones));
            pcb = pcb_create(instrucciones, siguiente_pid(), consola_fd);
            safe_pcb_push(cola_new_pcbs, pcb, cola_new_pcbs_mutex);
            log_info(kernel_logger, "Se crea el proceso %d en NEW", pcb->pid);
			sem_post(&procesos_new);
        } else {
			error_show("Mensaje desconocido");
		}
	}
}

void* rajar_pcb(void* arg) {
	while(1) {
		sem_wait(&procesos_finalizados);
		t_pcb* pcb = queue_pop(cola_exit_pcbs);
        pthread_mutex_lock(&pid_mutex);
        --pid_actual;
        pthread_mutex_unlock(&pid_mutex);
		log_debug(kernel_logger,"PCB con id: %d ha finalizado.",pcb->pid);
		sem_post(&multiprogramacion);
	}
}

/* Corto Plazo */

void corto_plazo_init(){
    cola_ready_FIFO_pcbs = queue_create();
	cola_ready_RR_pcbs = queue_create();
    pthread_t th_ejecucion;
	pthread_t th_transiciones_ready;

    sem_init(&procesos_ready,0,0);
    pthread_mutex_init(&cola_ready_RR_pcbs_mutex, NULL);
	pthread_mutex_init(&cola_ready_FIFO_pcbs_mutex, NULL);

    pthread_create(&th_ejecucion, NULL, &planificar_ejecucion, NULL);
	pthread_create(&th_transiciones_ready, NULL, &transicion_proceso_a_ready, NULL);

    pthread_detach(th_transiciones_ready);
	pthread_detach(th_ejecucion);
}

void* planificar_ejecucion(void* arg){ 
	while(1){
		t_pcb *pcb = seleccionar_pcb();
        planificar_interrupcion(pcb);
        enviar_pcb(pcb, conexion_cpu_dispatch);
        pcb_destroy(pcb);
        pcb = obtener_proceso_ejecutado();
        analizar_contexto_recibido(pcb);
        dirigir_proceso_ejecutado(pcb);
	}
}

t_pcb* seleccionar_pcb(){
    sem_wait(&procesos_ready);
    t_pcb *pcb = pop_ready_pcb();
    log_debug(kernel_logger,"Estado PCB: %d",pcb->estado);
    cambiar_estado(pcb, EXEC);
    log_debug(kernel_logger, "Enviando PCB");
    return pcb;
}

void planificar_interrupcion(t_pcb* pcb){
    if(pcb->con_desalojo){
        log_debug(kernel_logger, "Preparando interrupcion");
        iniciar_interrupcion();
        sem_post(&interrupcion_quantum);
    }
}

t_pcb* obtener_proceso_ejecutado(){
    t_pcb *pcb;
    int cod_op = recibir_operacion(conexion_cpu_dispatch);
    if(cod_op == PCB) {
        pcb = recibir_pcb(conexion_cpu_dispatch);
        char* pcb_string = pcb_to_string(pcb);
        //log_debug(kernel_logger,"PCB Recibida\n: %s", pcb_string);
        free(pcb_string);
        return pcb;
    }
    else {
        error_show("Error al recibir PCB");
        exit(EXIT_FAILURE);
    }  
}

void analizar_contexto_recibido(t_pcb* pcb){
    if(pcb->con_desalojo) {
		pthread_cancel(th_timer);
		log_debug(kernel_logger, "Interrupcion cancelada por vuelta del PCB");
	}
	if(pcb->interrupcion){ // cuando hay fin de quantum
		log_info(kernel_logger,"PID: %d - Desalojado por fin de Quantum", pcb->pid);
		pcb->interrupcion = false;
	}
}

void dirigir_proceso_ejecutado(t_pcb* pcb){ // corto plazo // tener en cuenta page default ya que no deberiamos modificar el program counter
	int ultima_instruccion_idx = pcb->program_counter - 1;

	instruccion* ultima_instruccion = list_get(pcb->instrucciones,ultima_instruccion_idx);
	switch(ultima_instruccion->operacion){
		case EXIT:
            solicitar_finalizacion(pcb);
			break;
		case IO:
            solicitar_io(pcb, ultima_instruccion);
			break;
		default:
            pasar_a_ready(pcb);
			break;
	}
}

void solicitar_io(t_pcb* pcb, instruccion* ultima_instruccion) {
	char* dispositivo = ultima_instruccion->parametro1;

	if(string_equals_ignore_case(dispositivo, "TECLADO") || string_equals_ignore_case(dispositivo, "PANTALLA")) {
		pthread_t th_solicitud_consola;
        pthread_create(&th_solicitud_consola, NULL, &solicitar_io_consola, (void*)pcb);
        pthread_detach(th_solicitud_consola);
	}
	else {
		solicitar_dispositivo(pcb, ultima_instruccion);
	}
}

// Cuerpo de la funcion que ejecuta el hilo para teclado/pantalla
// void* solicitar_io_consola(pcb){

//     obtenerultimains(pcb)

//     solicitud(pcb->instruccion->param1, consola_fd)

//     esperarrespuesta(consola_fd)

//     ---> pasaraready() <---

// }

// La funcion que se comunica con la consola
// void solicitud(instruccion, consola_fd) {
//     param1 = instruccion->param1
//     param2 = instruccion->param2
//     if(param1 == TECLADO) {
//         enviar_datos(teclado)
//     }
//     else {
//         enviar_valor_a_imprimir(pantalla, param2)
//     }
// }


void* transicion_proceso_a_ready(void* arg){
	while(1){
		sem_wait(&procesos_new);
        sem_wait(&multiprogramacion);
        t_pcb* pcb = safe_pcb_pop(cola_new_pcbs, cola_new_pcbs_mutex);
        pasar_a_ready(pcb);
		// solicitar_crear_estructuras_administrativas(pcb);

	}
}
// todo mauro
// solicitar_crear_estructuras_administrativas(pcb) {
// 	enviar_mensaje("pido crear estructuras administrativas", conexion_memoria);
// 	cod_mensaje mensaje = recibir_operacion(conexion_memoria);
// 	puts("Recibi la operacion");
// 	if(mensaje == MENSAJE){
// 		recibir_mensaje(memoria_logger, conexion_memoria);
// 	}
// }


// iniciar_conexion_con_memoria() {
// 	conexion_memoria = crear_conexion(kernel_config->ip_memoria, kernel_config->puerto_memoria);
// 	if(conexion_memoria != -1){
// 		log_debug(kernel_logger, "Conexion creada correctamente con MEMORIAs");
// 	}
// }

/* Planificacion Utils */

void cambiar_estado(t_pcb* pcb, estado_proceso nuevo_estado){
	char* nuevo_estado_string = strdup(estado_to_string(nuevo_estado));
	char* estado_anterior_string = strdup(estado_to_string(pcb->estado));
	log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_anterior_string, nuevo_estado_string);
	pcb->estado = nuevo_estado;
	free(estado_anterior_string);
	free(nuevo_estado_string);
}

void pasar_a_ready(t_pcb* pcb){
    push_ready_pcb(pcb);
    cambiar_estado(pcb, READY);
    sem_post(&procesos_ready);
}

void solicitar_finalizacion(t_pcb* pcb){
    cambiar_estado(pcb, FINISH_EXIT);
    queue_push(cola_exit_pcbs,pcb);
    sem_post(&procesos_finalizados);
}

void iniciar_interrupcion() {
 	pthread_create(&th_timer, NULL, &enviar_interrupt, NULL);
 	pthread_detach(th_timer);
 }

void* enviar_interrupt(void* arg){
	while(1) {
		sem_wait(&interrupcion_quantum);
		cod_mensaje mensaje = INTERRUPCION;
		log_debug(kernel_logger,"Hola! Soy hilo cpu interrupt");
		ejecutar_espera(kernel_config->quantum_RR);
		log_debug(kernel_logger,"Finalizo Espera");
		enviar_datos(conexion_cpu_interrupt,&mensaje, sizeof(mensaje));
		log_debug(kernel_logger, "Se envia mensaje de interrupcion a cpu \n");
	}
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

	if((algoritmo == FEEDBACK && !queue_is_empty(cola_ready_RR_pcbs)) || algoritmo == RR){
        pcb = safe_pcb_pop(cola_ready_RR_pcbs, cola_ready_RR_pcbs_mutex);
	} else {
		pcb = safe_pcb_pop(cola_ready_FIFO_pcbs, cola_ready_FIFO_pcbs_mutex);
	}
	return pcb;
}

void push_ready_pcb(t_pcb* pcb){
	if((algoritmo == FEEDBACK && (pcb->estado == NEW || pcb->estado == BLOCK)) || algoritmo == RR) {
		pcb->con_desalojo = true;
        safe_pcb_push(cola_ready_RR_pcbs, pcb, cola_ready_RR_pcbs_mutex);
	} else {
        safe_pcb_push(cola_ready_FIFO_pcbs, pcb, cola_ready_FIFO_pcbs_mutex);
	}
}

/* Utils */

void safe_pcb_push(t_queue* queue, t_pcb* pcb, pthread_mutex_t mutex){
    pthread_mutex_lock(&mutex);
    queue_push(queue, pcb);
    pthread_mutex_unlock(&mutex);
}

t_pcb* safe_pcb_pop(t_queue* queue, pthread_mutex_t mutex){
    t_pcb* pcb;
    pthread_mutex_lock(&mutex);
    pcb = queue_pop(queue);
    pthread_mutex_unlock(&mutex);
    return pcb;
}

// TODO: mover a static

// Interrupcion por Quantum primero debo generar la conversion entre milisegundos a 
// microsegundos, luego generar un proceso que espere recibir  quantum y generar un hilo 
// por interrupcion - esto nos va a facilitar eliminarlo en caso de que sobre quantum
// 
//ejecutarEspera le pasas el tiempo en milisegundos y el tipo te frena todo por ese tiempo
void ejecutar_espera(uint32_t tiempo){
	printf("%d milisegundos \n", (int) tiempo);
	usleep(tiempo * 1000);
}