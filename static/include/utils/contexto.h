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
#include <commons/string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define PAGE_FAULT -2
#define SEG_FAULT -3

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
	FIFO, RR, FEEDBACK
} t_algoritmo;

typedef enum{
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT,
	UNKNOWN_OP
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
	uint32_t nro_segmento;
	uint32_t tamanio_segmento;
	uint32_t indice_tabla_paginas;
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
	uint32_t pid;
	uint32_t program_counter;
	estado_proceso estado;
	int socket_consola;
	bool interrupcion;
	t_list * instrucciones;
	t_list* tabla_de_segmentos;
	t_list* tamanio_segmentos;
	registros_de_proposito_general registros;
	bool con_desalojo;
	bool page_fault;
	bool segmentation_fault;
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
	int indice_tabla_de_pagina;
	int numero_pagina;
}t_pagina;

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

t_pcb* pcb_create(t_proceso*, uint32_t, int);
cod_operacion string_to_cod_op(char*);
void pcb_destroy(void*);
void pcb_memoria_destroy(t_pcb_memoria*);
char* estado_to_string(estado_proceso);
char* pcb_to_string(t_pcb*);
char* instruccion_to_string(instruccion*);
void instruccion_destroy(void*);
char* operacion_to_string(cod_operacion);
void set_valor_registro(t_pcb*, char*, char*);
uint32_t obtener_valor_del_registro(t_pcb*, char*);
void ejecutar_espera(uint32_t);

#endif /* INCLUDE_UTILS_CONTEXTO_H_ */