#ifndef KERNEL_INCLUDE_PLANIFICACION_H_
#define KERNEL_INCLUDE_PLANIFICACION_H_

#include <kernel_utils.h>

void planificacion_init();

/* Largo plazo */
void largo_plazo_init();
void* atender_nueva_consola(void*);
void solicitar_creacion_estructuras_administrativas(t_pcb*);
t_list* crear_tabla_segmentos(t_list*, t_list*);
void* transicion_new_a_ready(void*);
void solicitar_finalizacion(t_pcb*);
void* rajar_pcb(void*);

/* Corto plazo */
void corto_plazo_init();
void pasar_a_ready(t_pcb*);
void* planificar_ejecucion(void*);
t_pcb* seleccionar_pcb();
void planificar_interrupcion(t_pcb*);
t_pcb* obtener_proceso_ejecutado();
void analizar_contexto_recibido(t_pcb*);
void dirigir_proceso_ejecutado(t_pcb*);
void* manejar_page_fault(void*);
void iniciar_interrupcion();
void* enviar_interrupt(void*);

#endif /* KERNEL_INCLUDE_PLANIFICACION_H_ */