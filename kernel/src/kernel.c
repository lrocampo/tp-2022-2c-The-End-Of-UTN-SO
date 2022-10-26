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

	planificacion_init(/*kernel_config*/);

	esperar_conexiones();

	liberar_conexion(conexion_cpu_dispatch);
	liberar_conexion(conexion_cpu_interrupt);
	
	log_debug(kernel_logger,"Termino Kernel");

	return EXIT_SUCCESS;
}

u_int32_t siguiente_pid(){
	u_int32_t siguiente_pid = 0;
	pthread_mutex_lock(&pid_mutex);
	siguiente_pid = ++pid_actual;
	pthread_mutex_unlock(&pid_mutex);
	return siguiente_pid;
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

void* atender_cpu_dispatch(void* arg){ // corto plazo
	while(1){
		// planificar
		sem_wait(&consolas);
		t_pcb *pcb = pop_ready_pcb();
		log_debug(kernel_logger,"Estado PCB: %d",pcb->estado);
		cambiar_estado(pcb, EXEC);

		enviar_pcb(pcb, conexion_cpu_dispatch);
		// interrumpir ? if(pcb->con_desalojo) signal(interrupcion_quantum)
		pcb_destroy(pcb);
		int cod_op = recibir_operacion(conexion_cpu_dispatch);
		if(cod_op == PCB) {
			pcb = recibir_pcb(conexion_cpu_dispatch);
			char* pcb_string = pcb_to_string(pcb);
			log_debug(kernel_logger,"PCB Recibida\n: %s", pcb_string);
			free(pcb_string);
		}
		else {
			error_show("Error.");
		}
		dirigir_pcb(pcb);
		//pcb_destroy(pcb);
	}
}


void* atender_cpu_interrupt(void* arg){
	puts("Hola! Soy hilo cpu interrupt");
	
	/*
	
	op_code codigo = MENSAJE;
	enviar_datos(socket_interrupt, &codigo, sizeof(codigo));
	log_info(kernel_logger_info, "Se envia mensaje de interrupcion a cpu \n");
	
	*/
}

void* rajar_pcb(void* arg) {
	while(1) {
		sem_wait(&procesos_finalizados);
		t_pcb* pcb = queue_pop(cola_exit_pcbs);
		log_debug(kernel_logger,"PCB con id: %d ha finalizado.",pcb->pid);
		sem_post(&multiprogramacion);
	}
}

void colas_init() {
	cola_new_pcbs = queue_create();
	cola_exit_pcbs = queue_create();
	// cola_ready_pcbs = queue_create();
	cola_ready_FIFO_pcbs = queue_create();
	cola_ready_RR_pcbs = queue_create();

}

void semaforos_init() {
	sem_init(&conexiones,0,0);
	sem_init(&consolas,0,0);
	sem_init(&multiprogramacion,0,kernel_config->grado_multiprogramacion);
	sem_init(&procesos_finalizados, 0, 0);
	pthread_mutex_init(&pid_mutex, NULL);
	// pthread_mutex_init(&cola_consolas_mutex, NULL);
	// pthread_mutex_init(&cola_ready_pcbs_mutex, NULL);
	pthread_mutex_init(&cola_ready_RR_pcbs_mutex, NULL);
	pthread_mutex_init(&cola_ready_FIFO_pcbs_mutex, NULL);

}

void threads_init() {
	pthread_t thread_cpu_dispatch;
	pthread_t thread_cpu_interrupt;
	pthread_t thread_consola;
	pthread_t thread_rajar_pcb;
	// corto
	pthread_create(&thread_cpu_dispatch, NULL, &atender_cpu_dispatch, NULL);
	pthread_create(&thread_cpu_interrupt, NULL, &atender_cpu_interrupt, NULL);
	pthread_create(&thread_consola, NULL, &atender_consolas, NULL);

	// largo
	pthread_create(&thread_rajar_pcb, NULL, &rajar_pcb, NULL);
	pthread_detach(thread_consola);
	pthread_detach(thread_cpu_dispatch);
	pthread_detach(thread_rajar_pcb);
}

void planificacion_init(/*t_kernel_config* kernel_config*/) {
	
	colas_init();

	semaforos_init();
	
	/* Al encender el kernel, arrancamos con pid 0 */
	pid_actual = 0;

	threads_init();
}

void iterator(instruccion* value) {
	log_debug(kernel_logger,"%d", value->operacion);
	log_debug(kernel_logger,"%s", value->parametro1);
	log_debug(kernel_logger,"%s", value->parametro2);
}

void dirigir_pcb(t_pcb* pcb){ // corto plazo

	int ultima_instruccion_idx = pcb->program_counter - 1;

	instruccion* ultima_instruccion = list_get(pcb->instrucciones,ultima_instruccion_idx);
	switch(ultima_instruccion->operacion){
		case EXIT:
			cambiar_estado(pcb, FINISH_EXIT);
			queue_push(cola_exit_pcbs,pcb);
			//log_info(kernel_logger,"PID: %d - Estado Anterior: EXEC - Estado Actual: EXIT", pcb->pid);
			sem_post(&procesos_finalizados);
			break;
		case IO:
			pcb->estado = BLOCK;
			// manejar IO
			// queue_push(cola_io, pcb)
			// signal(peticiones_io)
			break;
		default:
			if(pcb->interrupcion){ // cuando hay fin de quantum
				push_ready_pcb(pcb);
				cambiar_estado(pcb, READY);
				// manejar quantum
				log_info(kernel_logger,"PID: %d - Estado Anterior: EXEC - Estado Actual: BLOCK", pcb->pid);
			} else {
				push_ready_pcb(pcb);
				cambiar_estado(pcb, READY);
				// pcb->estado = READY;
				// queue_push(cola_ready_pcbs,pcb);
				//log_info(kernel_logger,"PID: %d - Estado Anterior: EXEC - Estado Actual: READY", pcb->pid);
			}
			break;
	}
}

/* planificador(pcb){

}
*/

void esperar_conexiones(){
	cola_consolas = queue_create();
	while(1){
		log_debug(kernel_logger,"Soy Kernel. Esperando conexion...");
		int consola_fd = esperar_cliente(kernel_server_fd);
		log_debug(kernel_logger, "se conecto un cliente");
		pthread_mutex_lock(&cola_consolas_mutex);
		queue_push(cola_consolas, (void*) (intptr_t) consola_fd);
		pthread_mutex_unlock(&cola_consolas_mutex);
		// mutex
		sem_post(&conexiones);
	}
}

// void agregar_pcb_a_ready(t_pcb* pcb) {
// 	pcb->estado = READY;
// 	pthread_mutex_lock(&cola_ready_pcbs_mutex);
// 	queue_push(cola_ready_pcbs, pcb);
// 	pthread_mutex_unlock(&cola_ready_pcbs_mutex);
// }

void* atender_consolas(void* arg){ // planificador_largo_plazo_nuevo
	t_pcb* pcb;
	t_list* instrucciones;
	while(1){
		sem_wait(&conexiones);
		pthread_mutex_lock(&cola_consolas_mutex);
		int consola_fd = (int) (intptr_t) queue_pop(cola_consolas);
		pthread_mutex_unlock(&cola_consolas_mutex);
		int cod_op = recibir_operacion(consola_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(kernel_logger, consola_fd);
				break;
			case PAQUETE_INSTRUCCIONES:
				instrucciones = recibir_paquete_con_funcion(consola_fd, deserializar_instruccion);
				log_debug(kernel_logger, "Recibí %d instrucciones", list_size(instrucciones));
				list_iterate(instrucciones, (void*) iterator);
				pcb = pcb_create(instrucciones, siguiente_pid(), consola_fd);
				// Agregar pcb a cola new
				queue_push(cola_new_pcbs,pcb);
				log_info(kernel_logger,"Se crea el proceso %d en NEW", pcb->pid);
				// Si el grado de multiprogramacion lo permite, lo pasa a ready
				sem_wait(&multiprogramacion);
				t_pcb* pcb = queue_pop(cola_new_pcbs);
				push_ready_pcb(pcb);
				cambiar_estado(pcb, READY);
				//agregar_pcb_a_ready(pcb);
				sem_post(&consolas);
				break;
			case -1:
				log_debug(kernel_logger, "Conexion con consola finalizada");
				return NULL;
			default:
				error_show("Operacion desconocida");
				break;
		}
	}
}


/*
	manejar_io(){
		while (1){
			
			checkear(io)

		}
	}

*/

t_pcb* pop_ready_pcb(){
	t_pcb* pcb;

	if((kernel_config->algoritmo == FEEDBACK
	&& !queue_is_empty(cola_ready_RR_pcbs))
	|| kernel_config->algoritmo == RR){
		pthread_mutex_lock(&cola_ready_RR_pcbs_mutex);
		pcb = queue_pop(cola_ready_RR_pcbs);
		pthread_mutex_unlock(&cola_ready_RR_pcbs_mutex);
	} else {
		pthread_mutex_lock(&cola_ready_FIFO_pcbs_mutex);
		pcb = queue_pop(cola_ready_FIFO_pcbs);
		pthread_mutex_unlock(&cola_ready_FIFO_pcbs_mutex);
	}

	return pcb;

}

void push_ready_pcb(t_pcb* pcb){
	if((kernel_config->algoritmo == FEEDBACK 
	&& (pcb->estado == NEW || pcb->estado == BLOCK)) // agregar io
	|| kernel_config->algoritmo == RR) {
		pthread_mutex_lock(&cola_ready_RR_pcbs_mutex);
		queue_push(cola_ready_RR_pcbs,pcb);
		pthread_mutex_unlock(&cola_ready_RR_pcbs_mutex);
	} else {
		pthread_mutex_lock(&cola_ready_FIFO_pcbs_mutex);
		queue_push(cola_ready_FIFO_pcbs,pcb);
		pthread_mutex_unlock(&cola_ready_FIFO_pcbs_mutex);
	}
	
}

void cambiar_estado(t_pcb* pcb, estado_proceso nuevo_estado){
	log_info(kernel_logger,"PID: %d - Estado Anterior: %d - Estado Actual: %d", pcb->pid, pcb->estado, nuevo_estado);
	pcb->estado = nuevo_estado;
}

// void colas_init(t_kernel_config* kernel_config){
	
// 	//cola_ready_FIFO_pcbs = queue_create();
// 	//cola_ready_RR_pcbs = queue_create();
// }


// Preguntas checkpint

// De quien es responsabilidad cambiar el estado de un pcb? Dentro de que funcion?
// Como se supone que vamos a abstraer y separar la plani de largo y la plani de corto?
// Por que a veces los modulos se conectan bien y otras veces mal.
// Hay que detachear los demas threads? cpu dispatch, cpu interrupt, rajar_pcb?
// Como se espera (en que orden) que se levanten los modulos? Para la entrega final.
// mediano plazo


// void* atender_consola(void* p){
// 	int consola_fd = *(int *) p;
// 	free(p);
// 	t_pcb* pcb;
// 	t_list* instrucciones;
// 		while(1){
		
// 	}
// }
