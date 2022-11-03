#ifndef KERNEL_INCLUDE_KERNEL_UTILS_H_
#define KERNEL_INCLUDE_KERNEL_UTILS_H_

#include <utils/comunicacion.h>
#include <utils/socket.h>
#include <utils/logger.h>
#include <utils/utiles_config.h>
#include <utils/contexto.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <kernel.h>

extern t_queue* cola_new_pcbs;
extern t_queue* cola_exit_pcbs;
extern t_queue* cola_ready_FIFO_pcbs;
extern t_queue* cola_ready_RR_pcbs;

extern sem_t procesos_ready;
extern sem_t procesos_new;
extern sem_t multiprogramacion;
extern sem_t procesos_finalizados; 
extern sem_t interrupcion_quantum;
extern sem_t* s_dispositivos_io;

extern uint32_t pid_actual;
extern t_algoritmo algoritmo;

extern pthread_mutex_t pid_mutex;
extern pthread_mutex_t cola_ready_RR_pcbs_mutex;
extern pthread_mutex_t cola_new_pcbs_mutex;
extern pthread_mutex_t cola_ready_FIFO_pcbs_mutex;
extern pthread_mutex_t* cola_dispositivo_mutex;

extern pthread_t th_timer;
extern pthread_t th_conexiones;

/* Largo plazo */
void largo_plazo_init();
void* atender_nueva_consola(void*);
void* rajar_pcb(void*);

/* Corto plazo */
void corto_plazo_init();
void* planificar_ejecucion(void*);
t_pcb* seleccionar_pcb();
void planificar_interrupcion(t_pcb*);
t_pcb* obtener_proceso_ejecutado();
void analizar_contexto_recibido(t_pcb*);
void dirigir_proceso_ejecutado(t_pcb*);
void* transicion_proceso_a_ready(void*); // transicion_new_a_ready
// TODO: transicion_block_a_ready

/* Planificacion Utils */
void cambiar_estado(t_pcb*, estado_proceso);
void pasar_a_ready(t_pcb*);
void solicitar_finalizacion(t_pcb*);
void iniciar_interrupcion();
void* enviar_interrupt(void*);
u_int32_t siguiente_pid();
void push_ready_pcb(t_pcb*);
t_pcb* pop_ready_pcb();

/* Utils */
void safe_pcb_push(t_queue*, t_pcb*, pthread_mutex_t);
t_pcb* safe_pcb_pop(t_queue*, pthread_mutex_t);


/* IO */
void* ejecucion_io(void*);
void solicitar_dispositivo(t_pcb*,instruccion*);
void solicitar_io(t_pcb*, instruccion*);
void dispositivos_io_init();
t_dispositivo* obtener_dispositivo_por_nombre(char*);
instruccion* obtener_ultima_instruccion(t_pcb* pcb);
void solicitar_dispositivo(t_pcb*, instruccion*);

#endif /* KERNEL_INCLUDE_KERNEL_UTILS_H_ */