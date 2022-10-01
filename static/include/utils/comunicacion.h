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
#include<unistd.h>
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
	PAQUETE_INSTRUCCIONES,
	PCB
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
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
void recibir_mensaje(t_log*,int);
void agregar_a_paquete_con_header(t_paquete*, void*, int);

/* NUEVAS */
t_paquete* new_paquete_con_codigo_de_operacion(int);
void empaquetar_instrucciones(t_list*, t_paquete*);
void enviar_instrucciones(t_list*, int);
void serializar_instruccion(instruccion*,t_paquete*);
void agregar_valor_a_paquete(t_paquete* , void* , int );
void* deserializar_instruccion(void*, int*);
t_list* recibir_paquete_con_funcion(int, void* (*funcion_deserializar)(void*,int*));
void enviar_pcb(t_pcb*, int);
void empaquetar_pcb(t_pcb*, t_paquete*);
void empaquetar_tabla_segmentos(tabla_de_segmentos, t_paquete*);
void empaquetar_registros(registros_de_proposito_general, t_paquete*);
t_pcb* recibir_pcb(int);



/* NO SE USA */
// t_list* deserializar_instrucciones(int);
//t_list* recibir_paquete(int);


#endif /* INCLUDE_UTILS_COMUNICACION_H_ */