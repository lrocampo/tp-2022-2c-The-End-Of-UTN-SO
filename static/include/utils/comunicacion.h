/*
 * comunicacion.h
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#ifndef INCLUDE_SOCKET_COMUNICACION_H_
#define INCLUDE_SOCKET_COMUNICACION_H_

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<string.h>

typedef enum
{
	MENSAJE,
	PAQUETE
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

void enviar_mensaje(char*, int);
void recibir_mensaje(int);
void* recibir_buffer(int*, int);
void* serializar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
int recibir_operacion(int);

#endif /* INCLUDE_SOCKET_COMUNICACION_H_ */