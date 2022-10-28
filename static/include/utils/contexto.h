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


typedef struct {
	char* dispositivo;
	int unidades_de_trabajo;
	uint32_t registro_de_io;
} contexto_io;

typedef enum{
	NEW,
	READY,
	EXEC,
	BLOCK,
	FINISH_EXIT,
	FINISH_ERROR,
	UNKNOWN_STATE
} estado_proceso;

typedef enum{
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT,
	UNKNOWN_OP
} cod_operacion;

typedef enum{
	SOLICITUD_FINALIZAR,
	SOLICITUD_TECLADO,
	SOLICITUD_PANTALLA
} respuesta;

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
	contexto_io contexto_de_io;
}t_pcb;

t_pcb* pcb_create(t_list*, uint32_t, int);
cod_operacion string_to_cod_op(char*);
void pcb_destroy(t_pcb*);
char* estado_to_string(estado_proceso);
char* pcb_to_string(t_pcb* pcb);
char* instruccion_to_string(instruccion*);
void instruccion_destroy(void*);

#endif /* INCLUDE_UTILS_CONTEXTO_H_ */