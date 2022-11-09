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
}tabla_de_segmentos;

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
	tabla_de_segmentos tabla;
	registros_de_proposito_general registros;
	bool con_desalojo;
}t_pcb;

typedef struct {
	int numero_pagina;
	int indice_tabla_de_pagina;
	bool presencia;
	bool modificado;
	bool uso;
	int posicion_swap;
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
	tabla_de_segmentos tabla;
} t_pcb_memoria;

t_pcb* pcb_create(t_list*, uint32_t, int);
cod_operacion string_to_cod_op(char*);
void pcb_destroy(void*);
char* estado_to_string(estado_proceso);
char* pcb_to_string(t_pcb*);
char* instruccion_to_string(instruccion*);
void instruccion_destroy(void*);
char* operacion_to_string(cod_operacion);
void set_valor_registro(t_pcb*, char*, char*);
uint32_t obtener_valor_del_registro(t_pcb*, char*);

#endif /* INCLUDE_UTILS_CONTEXTO_H_ */