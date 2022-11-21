/*
 * contexto.h
 *
 *  Created on: Sep 21, 2022
 *      Author: utnso
 */

#ifndef INCLUDE_UTILS_CONTEXTO_H_
#define INCLUDE_UTILS_CONTEXTO_H_

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <commons/string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
	uint32_t ax;
	uint32_t bx;
	uint32_t cx;
	uint32_t dx;
}registros_de_proposito_general;

typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCK,
	FINISH_EXIT,
	FINISH_ERROR,
	UNKNOWN_STATE
} estado_proceso;

typedef enum {
	FIFO, RR, FEEDBACK, CLOCK, CLOCK_M, LRU
} t_algoritmo;

typedef enum{
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT,
	UNKNOWN_OP,
	ERROR_MEMORIA
} cod_operacion;

typedef struct {
	cod_operacion operacion;
	char* parametro1;
	char* parametro2;
} instruccion;

typedef struct {
	int indice;
	char* nombre;
	int duracion;
	t_queue* cola;
} t_dispositivo;

typedef struct {
	int nro_segmento;
	int tamanio_segmento;
	int indice_tabla_paginas;
}t_segmento;

typedef struct 
{
	t_list* instrucciones;
	t_list* segmentos;
} t_proceso;

// voy a devolver el pcb, por que?
// Me interrumpiste
// Entrada y salida
// Termine
typedef struct {
	int indice_tabla_de_pagina;
	int numero_pagina;
}t_pagina;
typedef struct {
	int pid;
	int program_counter;
	estado_proceso estado;
	int socket_consola;
	bool interrupcion;
	t_list * instrucciones;
	t_list* tabla_de_segmentos;
	t_list* tamanio_segmentos;
	registros_de_proposito_general registros;
	t_pagina* pagina_fault;
	bool con_desalojo;
	bool page_fault;
	bool segmentation_fault;
	int direccion_fisica;
}t_pcb;

typedef struct {
	int marco;
	int segmento;
	bool presencia;
	bool modificado;
	bool uso;
	int posicion_swap;
}t_entrada_tp;

typedef struct {
	int indice_tabla_de_pagina;
	int pid;
	t_list* entradas;
}t_tabla_de_paginas;

typedef struct {
	int numero_marco;
	int pid;
} t_marco;

typedef struct {
	int numero_marco;
	int offset;
} t_direccion_fisica;

typedef struct {
	int pid;
	t_list* segmentos;
} t_pcb_memoria;

typedef struct {
    int pid;
    int segmento;
    int pagina;
    int marco;
    int instante_de_ultima_referencia;
} t_entrada_tlb;

t_dispositivo* dispositivo_io_create(int, char**, char**);
void proceso_destroy(t_proceso*);
t_proceso* proceso_create(t_list*, t_list*);
void dispositivo_io_destroy(void*);
t_list* pcb_queue_to_pid_list(t_queue*);
t_pcb* pcb_create(t_proceso*, int, int);
t_pagina* pagina_create(int indice_tabla_paginas, int numero_pagina);
t_marco* marco_create(int pid, int numero_marco);
cod_operacion string_to_cod_op(char*);
char* algoritmo_to_string(t_algoritmo);
char* list_to_string(t_list*);
void pcb_destroy(void*);
void pcb_memoria_destroy(t_pcb_memoria*);
char* estado_to_string(estado_proceso);
char* pcb_to_string(t_pcb*);
t_segmento* segmento_create(int, int, int);
char* instruccion_to_string(instruccion*);
instruccion* instruccion_create(cod_operacion, char*, char*);
void instruccion_destroy(void*);
char* operacion_to_string(cod_operacion);
void set_valor_registro(t_pcb*, char*, char*);
uint32_t obtener_valor_del_registro(t_pcb*, char*);
void ejecutar_espera(int);

#endif /* INCLUDE_UTILS_CONTEXTO_H_ */