/*
 * comunicacion.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#include <utils/comunicacion.h>

void enviar_mensaje(char *mensaje, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
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

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
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

void enviar_instrucciones(t_list *instrucciones, int socket_cliente)
{
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(PAQUETE);

	empaquetar_instrucciones(instrucciones, paquete);

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

t_list *deserializar_instrucciones(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;
	buffer = recibir_buffer(&size, socket_cliente);

	while (desplazamiento < size)
	{
		instruccion *nueva_instruccion = malloc(sizeof(instruccion));
		puts("nueva instruccion");

		memcpy(&(nueva_instruccion->operacion), buffer + desplazamiento, sizeof(cod_operacion));
		desplazamiento += sizeof(cod_operacion);

		puts(string_itoa(nueva_instruccion->operacion));

		memcpy(&(tamanio), buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		puts(string_itoa(tamanio));

		nueva_instruccion->parametro1 = malloc(tamanio);

		memcpy(nueva_instruccion->parametro1, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;

		memcpy(&(tamanio), buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		puts(string_itoa(tamanio));
		nueva_instruccion->parametro2 = malloc(tamanio);

		memcpy(nueva_instruccion->parametro2, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;

		puts(nueva_instruccion->parametro1);
		puts(nueva_instruccion->parametro2);

		list_add(valores, nueva_instruccion);
	}
	free(buffer);
	return valores;
}

// t_list* recibir_paquete(int socket_cliente)
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

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	puts(buffer);
	free(buffer);
}

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

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void new_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete *new_paquete_con_codigo_de_operacion(int codigo)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigo;
	new_buffer(paquete);
	return paquete;
}

void empaquetar_instrucciones(t_list *instrucciones, t_paquete *paquete)
{
	int cantidad_instrucciones = list_size(instrucciones);
	// memcpy(paquete->buffer->stream, &cantidad_instrucciones, sizeof(int)); // primero se copia el tamnio de la lista para despues ir agregando los otros campos de la lista
	// paquete->buffer->size += sizeof(int);

	for (int i = 0; i < cantidad_instrucciones; i++)
	{
		instruccion *instruccion = list_get(instrucciones, i);
		serializar_instruccion(instruccion, paquete);
	}
	puts("salimossss");
}

void serializar_instruccion(instruccion *instruccion, t_paquete *paquete)
{
	puts(string_itoa(instruccion->operacion));
	puts(instruccion->parametro1);
	// int parametro1_tamanio = string_length(instr->parametro1);
	// int parametro2_tamanio = string_length(instr->parametro2);
	// agregar_a_paquete(paquete, &(instruccion->operacion), sizeof(cod_operacion));
	// agregar_a_paquete(paquete, &(parametro1_tamanio), sizeof(int));
	agregar_valor_a_paquete(paquete, &(instruccion->operacion), sizeof(cod_operacion));
	agregar_a_paquete(paquete, instruccion->parametro1, strlen(instruccion->parametro1) + 1);
	// agregar_a_paquete(paquete, &(parametro2_tamanio), sizeof(int));
	agregar_a_paquete(paquete, instruccion->parametro2, strlen(instruccion->parametro1) + 1);
	free(instruccion);
}

void agregar_valor_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
	memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);

	paquete->buffer->size += tamanio;
}
