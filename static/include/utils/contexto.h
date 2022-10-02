/*
 * contexto.h
 *
 *  Created on: Sep 21, 2022
 *      Author: utnso
 */

#ifndef INCLUDE_UTILS_CONTEXTO_H_
#define INCLUDE_UTILS_CONTEXTO_H_

#include <commons/collections/list.h>
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
	FINISH_ERROR
} estado_proceso;

typedef enum{
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT
} cod_operacion;

typedef struct {
	cod_operacion operacion;
	char* parametro1;
	char* parametro2;
} instruccion;

typedef struct {
	uint32_t nro_segmento;
	uint32_t tamanio_segmento;
	uint32_t indice_tabla_paginas;
}tabla_de_segmentos;

typedef struct {
	uint32_t pid;
	uint32_t program_counter;
	estado_proceso estado;
	t_list * instrucciones;
	tabla_de_segmentos tabla;
	registros_de_proposito_general registros;
}t_pcb;

t_pcb* pcb_create(t_list*, uint32_t);
void pcb_destroy(t_pcb*);
char* estado_to_string(estado_proceso);
char* pcb_to_string(t_pcb* pcb);
char* instruccion_to_string(instruccion*);

#endif /* INCLUDE_UTILS_CONTEXTO_H_ */