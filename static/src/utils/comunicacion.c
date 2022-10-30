/*
 * comunicacion.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#include <utils/comunicacion.h>


/* OPERACIONES */

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

/* BUFFER */

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void new_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

/* MENSAJES */

void enviar_mensaje(char *mensaje, int socket_cliente)
{
	enviar_mensaje_con_codigo(mensaje, MENSAJE, socket_cliente);
}

void enviar_mensaje_con_codigo(char *mensaje, cod_mensaje codigo, int socket_cliente){
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_mensaje = codigo;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void recibir_mensaje(t_log* logger, int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_debug(logger,"Se recibio el siguiente mensaje: %s",buffer);
	free(buffer);
}

/* VALORES */


int recibir_valor(int socket_cliente){
	int size;
	int valor;
	char *buffer = recibir_buffer(&size, socket_cliente);
	valor = atoi(buffer);
	free(buffer);
	return valor;
}

void enviar_valor_con_codigo(int valor, cod_mensaje codigo, int socket_cliente){
	char* mensaje = string_itoa(valor);
	enviar_mensaje_con_codigo(mensaje, codigo, socket_cliente);
}

void enviar_valor_a_imprimir(int valor, int socket_cliente){
	enviar_valor_con_codigo(valor, PANTALLA, socket_cliente);
}

void enviar_valor_ingresado(int valor, int socket_cliente){
	enviar_valor_con_codigo(valor, TECLADO, socket_cliente);
}

/* PAQUETES */

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_mensaje), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

t_list* recibir_paquete_con_funcion(int socket_cliente, void* (*funcion_deserializar)(void*,int*))
{
	int desplazamiento = 0;
	int size;
	void * buffer;
	t_list* valores = list_create();
	void* valor;

	buffer = recibir_buffer(&size, socket_cliente);

	while(desplazamiento < size)
	{
		valor = funcion_deserializar(buffer, &desplazamiento);	 
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void agregar_a_paquete_con_header(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

t_paquete *new_paquete_con_codigo_de_operacion(int codigo)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_mensaje = codigo;
	new_buffer(paquete);
	return paquete;
}

void agregar_valor_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
	memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);

	paquete->buffer->size += tamanio;
}

/* PAQUETE_INSTRUCCIONES */

void enviar_instrucciones(t_list *instrucciones, int socket_cliente)
{
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(PAQUETE_INSTRUCCIONES);

	empaquetar_instrucciones(instrucciones, paquete);

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

void empaquetar_instrucciones(t_list *instrucciones, t_paquete *paquete)
{
	int cantidad_instrucciones = list_size(instrucciones);

	for (int i = 0; i < cantidad_instrucciones; i++)
	{
		instruccion *instruccion = list_get(instrucciones, i);
		serializar_instruccion(instruccion, paquete);
	}
}

void serializar_instruccion(instruccion *instruccion, t_paquete *paquete)
{
	agregar_valor_a_paquete(paquete, &(instruccion->operacion), sizeof(cod_operacion));
	agregar_a_paquete_con_header(paquete, instruccion->parametro1, strlen(instruccion->parametro1) + 1);
	agregar_a_paquete_con_header(paquete, instruccion->parametro2, strlen(instruccion->parametro2) + 1);
}

void* deserializar_instruccion(void* buffer, int* desplazamiento)
{
	instruccion* nueva_instruccion = malloc(sizeof(instruccion)); 
	int tamanio = 0;

	memcpy(&(nueva_instruccion->operacion), buffer + *desplazamiento, sizeof(cod_operacion));
	*desplazamiento += sizeof(cod_operacion);

	memcpy(&(tamanio), buffer + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	nueva_instruccion->parametro1 = malloc(tamanio);

	memcpy(nueva_instruccion->parametro1, buffer + *desplazamiento, tamanio);
	*desplazamiento += tamanio;

	memcpy(&(tamanio), buffer + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	nueva_instruccion->parametro2 = malloc(tamanio);

	memcpy(nueva_instruccion->parametro2, buffer + *desplazamiento, tamanio);
	*desplazamiento += tamanio;


	return nueva_instruccion;
}

/* PAQUETE_PCB */

void enviar_pcb(t_pcb* pcb, int socket_cliente)
{
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(PCB);

	empaquetar_pcb(pcb, paquete);

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

void empaquetar_pcb(t_pcb* pcb,t_paquete* paquete){
	int cantidad_instrucciones = list_size(pcb->instrucciones);
	agregar_valor_a_paquete(paquete, &(pcb->pid), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(pcb->program_counter), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(pcb->estado), sizeof(estado_proceso));
	agregar_valor_a_paquete(paquete, &(pcb->socket_consola), sizeof(int));
	agregar_valor_a_paquete(paquete, &(pcb->interrupcion), sizeof(bool));
	agregar_valor_a_paquete(paquete, &(pcb->con_desalojo), sizeof(bool));
	empaquetar_tabla_segmentos(pcb->tabla, paquete);
	empaquetar_registros(pcb->registros, paquete);
	agregar_valor_a_paquete(paquete, &(cantidad_instrucciones), sizeof(int));
	empaquetar_instrucciones(pcb->instrucciones, paquete);
}

void empaquetar_tabla_segmentos(tabla_de_segmentos tabla,t_paquete* paquete){
	agregar_valor_a_paquete(paquete, &(tabla.indice_tabla_paginas), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(tabla.nro_segmento), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(tabla.tamanio_segmento), sizeof(u_int32_t));
}

void empaquetar_registros(registros_de_proposito_general registros, t_paquete* paquete){
	agregar_valor_a_paquete(paquete, &(registros.ax), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(registros.bx), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(registros.cx), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(registros.dx), sizeof(u_int32_t));
}

t_pcb* recibir_pcb(int socket_cliente){
	int desplazamiento = 0;
	int size;
	int cantidad_instrucciones;
	void * buffer;
	t_list* lista_instrucciones = list_create();

	buffer = recibir_buffer(&size, socket_cliente);

	t_pcb* nueva_pcb = malloc(sizeof(t_pcb)); 

	memcpy(&(nueva_pcb->pid), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->program_counter), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->estado), buffer + desplazamiento, sizeof(estado_proceso));
	desplazamiento += sizeof(estado_proceso);

	memcpy(&(nueva_pcb->socket_consola), buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&(nueva_pcb->interrupcion), buffer + desplazamiento, sizeof(bool));
	desplazamiento += sizeof(bool);

	memcpy(&(nueva_pcb->con_desalojo), buffer + desplazamiento, sizeof(bool));
	desplazamiento += sizeof(bool);

	memcpy(&(nueva_pcb->tabla.indice_tabla_paginas), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->tabla.nro_segmento), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->tabla.tamanio_segmento), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.ax), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.bx), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.cx), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.dx), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(cantidad_instrucciones), buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	for(int i = 0; i < cantidad_instrucciones; i++){
		instruccion* nueva_instruccion = deserializar_instruccion(buffer, &desplazamiento);
		list_add(lista_instrucciones, nueva_instruccion);
		// free(nueva_instruccion->parametro1);
		// free(nueva_instruccion->parametro2);
		// TODO: FALTA LIBERAR MEMORIA NUEVA INSTRUCCIon
	}

	nueva_pcb->instrucciones = lista_instrucciones;
	free(buffer);
	return nueva_pcb;
}


/* NO SE USA (pero no borrar todavia)*/

// t_list* recibir_paquete(int socket_cliente, )
// {
// 	int size;
// 	int desplazamiento = 0;
// 	void * buffer;
// 	t_list* valores = list_create();
// 	int tamanio;

// 	buffer = recibir_buffer(&size, socket_cliente);
// 	while(desplazamiento < size)
// 	{
// 		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
// 		desplazamiento+=sizeof(int);
// 		char* valor = malloc(tamanio);
// 		memcpy(valor, buffer+desplazamiento, tamanio);
// 		desplazamiento+=tamanio;
// 		list_add(valores, valor);
// 	}
// 	free(buffer);
// 	return valores;
// }


// t_list *deserializar_instrucciones(int socket_cliente)
// {
// 	int size;
// 	int desplazamiento = 0;
// 	void *buffer;
// 	t_list *valores = list_create();
// 	int tamanio;
// 	buffer = recibir_buffer(&size, socket_cliente);

// 	while (desplazamiento < size)
// 	{
// 		instruccion *nueva_instruccion = malloc(sizeof(instruccion));
// 		puts("nueva instruccion");

// 		memcpy(&(nueva_instruccion->operacion), buffer + desplazamiento, sizeof(cod_operacion));
// 		desplazamiento += sizeof(cod_operacion);

// 		memcpy(&(tamanio), buffer + desplazamiento, sizeof(int));
// 		desplazamiento += sizeof(int);

// 		nueva_instruccion->parametro1 = malloc(tamanio);

// 		memcpy(nueva_instruccion->parametro1, buffer + desplazamiento, tamanio);
// 		desplazamiento += tamanio;

// 		memcpy(&(tamanio), buffer + desplazamiento, sizeof(int));
// 		desplazamiento += sizeof(int);

// 		nueva_instruccion->parametro2 = malloc(tamanio);

// 		memcpy(nueva_instruccion->parametro2, buffer + desplazamiento, tamanio);
// 		desplazamiento += tamanio;

// 		list_add(valores, nueva_instruccion);
// 	}
// 	free(buffer);
// 	return valores;
// }