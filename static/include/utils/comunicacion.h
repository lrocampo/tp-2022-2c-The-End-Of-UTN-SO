/*
 * comunicacion.h
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#ifndef INCLUDE_UTILS_COMUNICACION_H_
#define INCLUDE_UTILS_COMUNICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<commons/log.h>
#include<utils/contexto.h>

typedef enum
{
	MENSAJE,
	PAQUETE,
	PROCESO,
	PCB,
	INTERRUPCION,
	FINALIZAR,
	TECLADO,
	PANTALLA,
	ESTRUCTURAS,
	OKI_PANTALLA,
	OKI_TECLADO,
	OKI_ESTRUCTURAS
} cod_mensaje;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	cod_mensaje codigo_mensaje;
	t_buffer* buffer;
} t_paquete;

/* SE USA */

/* TP0 */
void enviar_mensaje(char*, int);
void* recibir_buffer(int*, int);
void* serializar_paquete(t_paquete*, int); 				
void eliminar_paquete(t_paquete*);
void enviar_paquete(t_paquete*, int);
int recibir_operacion(int);
void new_buffer(t_paquete*);

/* TP0 MODIFICADA */
void agregar_a_paquete_con_header(t_paquete*, void*, int);
void recibir_mensaje(t_log*,int);

/* NUEVAS */

t_paquete* new_paquete_con_codigo_de_operacion(int);

void agregar_valor_a_paquete(t_paquete* , void* , int );
void* deserializar_instruccion(int*, void*);
t_list* deserializar_paquete_mensaje(int*, void*);
t_list* deserializar_instrucciones(int*, void*);
t_list* deserializar_tabla_segmentos(int*, void*);
t_proceso* deserializar_proceso(int);

t_pcb* recibir_pcb(int);
t_pcb_memoria* recibir_pcb_memoria(int);
int recibir_valor(int);

void enviar_valor_con_codigo(int, cod_mensaje, int);
void enviar_valor_a_imprimir(int, int);
void enviar_valor_ingresado(int, int);
void enviar_mensaje_con_codigo(char *, cod_mensaje, int);
void* enviar_interrupt(void*);
void enviar_pcb(t_pcb*, int);
void enviar_pcb_memoria(t_pcb_memoria*, int);
void enviar_proceso(t_proceso*, int);
int enviar_datos(int , void *, uint32_t);

void empaquetar_instrucciones(t_list*, t_paquete*);
void empaquetar_pcb(t_pcb*, t_paquete*);
void empaquetar_tabla_segmentos(t_list*, t_paquete*);
void empaquetar_registros(registros_de_proposito_general, t_paquete*);
void empaquetar_proceso(t_proceso*,t_paquete*);
void empaquetar_segmentos(t_list*,t_paquete*);
void agregar_valor_a_paquete(t_paquete* , void* , int );
void serializar_instruccion(instruccion*,t_paquete*);

char* recibir_valor_string(int);
t_pcb* recibir_pcb(int);
int recibir_valor(int);
t_list* recibir_paquete_con_funcion(int, void* (*funcion_deserializar)(int*, void*));

#endif /* INCLUDE_UTILS_COMUNICACION_H_ */