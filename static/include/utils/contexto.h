/*
 * contexto.h
 *
 *  Created on: Sep 21, 2022
 *      Author: utnso
 */

#ifndef INCLUDE_UTILS_CONTEXTO_H_
#define INCLUDE_UTILS_CONTEXTO_H_

#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
	int operacion;
	char* parametro1;
	char* parametro2;
} instruccion;

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
	EXIT
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
	uint32_t nro_segmento;
	uint32_t tamanio_segmento;
	uint32_t indice_tabla_paginas;
}tabla_de_segmentos;

typedef struct {
	uint32_t pid;
	t_list * instrucciones;
	uint32_t program_counter;
	estado_proceso estado;
	tabla_de_segmentos tabla;
	registros_de_proposito_general registros;
}t_pcb;

t_pcb* pcb_create();
t_pcb* pcb_destroy();

#endif /* INCLUDE_UTILS_CONTEXTO_H_ */