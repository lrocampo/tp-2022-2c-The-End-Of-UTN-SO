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
	if (recv(socket_cliente, &cod_op, sizeof(cod_mensaje), MSG_WAITALL) > 0)
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

int recibir_datos(int socket_fd, void *dest, uint32_t size) {
	return recv(socket_fd, dest, size, 0); // cuantos bytes a recibir y a donde los quiero recibir
}

t_list* deserializar_paquete_mensaje(int* desplazamiento, void* buffer){
	int tamanio_string;
	int cantidad_elementos;
	t_list* lista_result = list_create();
	memcpy(&(cantidad_elementos), buffer + (*desplazamiento), sizeof(int));
	*desplazamiento += sizeof(int);
	for(int i=0; i < cantidad_elementos; i++)
	{
		memcpy(&tamanio_string, buffer + (*desplazamiento), sizeof(int));
		*desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio_string);
		memcpy(valor, buffer+(*desplazamiento), tamanio_string);
		*desplazamiento+=tamanio_string;
		list_add(lista_result, valor);
	}

	return lista_result;
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

char* recibir_valor_string(int socket_cliente){
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}

void enviar_valor_con_codigo(int valor, cod_mensaje codigo, int socket_cliente){
	char* mensaje = string_itoa(valor);
	enviar_mensaje_con_codigo(mensaje, codigo, socket_cliente);
	free(mensaje);
}

void enviar_valor_a_imprimir(int valor, int socket_cliente){
	enviar_valor_con_codigo(valor, PANTALLA, socket_cliente);
}

void enviar_valor_ingresado(int valor, int socket_cliente){
	enviar_valor_con_codigo(valor, OKI_TECLADO, socket_cliente);
}

int enviar_datos(int socket_fd, void *source, uint32_t size) {
	return send(socket_fd, source, size, 0);
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

t_list* recibir_paquete_con_funcion(int socket_cliente, void* (*funcion_deserializar)(int*, void*))
{
	int desplazamiento = 0;
	int size;
	int cantidad_elementos;
	void * buffer;
	t_list* valores = list_create();
	void* valor;

	buffer = recibir_buffer(&size, socket_cliente);

	memcpy(&(cantidad_elementos), buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	for(int i = 0; i < cantidad_elementos; i++)
	{
		valor = funcion_deserializar(&desplazamiento, buffer);	 
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

void empaquetar_instrucciones(t_list *instrucciones, t_paquete *paquete)
{
	int cantidad_instrucciones = list_size(instrucciones);
	agregar_valor_a_paquete(paquete, &(cantidad_instrucciones), sizeof(int));
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

void* deserializar_instruccion(int* desplazamiento, void* buffer)
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

t_list* deserializar_instrucciones(int* desplazamiento, void* buffer){
	t_list* lista_instrucciones = list_create();
	int cantidad_instrucciones;

	memcpy(&(cantidad_instrucciones), buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento) += sizeof(int);

	for(int i = 0; i < cantidad_instrucciones; i++){
		instruccion* instruccion = deserializar_instruccion(desplazamiento, buffer);	 
		list_add(lista_instrucciones, instruccion);
	}

	return lista_instrucciones;
}


/* PROCESO */

void enviar_proceso(t_proceso* proceso, int socket_cliente){
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(PROCESO);

	empaquetar_proceso(proceso, paquete);

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
	list_destroy_and_destroy_elements(proceso->instrucciones, instruccion_destroy);
	list_destroy_and_destroy_elements(proceso->segmentos, free);
}

void empaquetar_proceso(t_proceso* proceso,t_paquete* paquete){
	empaquetar_instrucciones(proceso->instrucciones, paquete);
	empaquetar_strings(proceso->segmentos, paquete);
}

t_proceso* deserializar_proceso(int socket_cliente){
	int desplazamiento = 0;
	int size;
	void * buffer;
	t_list* lista_instrucciones;
	t_list* lista_segmentos;

	buffer = recibir_buffer(&size, socket_cliente);

	t_proceso* proceso = malloc(sizeof(t_proceso));

	lista_instrucciones = deserializar_instrucciones(&desplazamiento, buffer);
	lista_segmentos = deserializar_paquete_mensaje(&desplazamiento, buffer);

	proceso->instrucciones = lista_instrucciones;
	proceso->segmentos = lista_segmentos;

	free(buffer);
	return proceso;
}

/* PAGINAS */

void enviar_pagina(t_pagina* pagina, int socket_cliente){
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(PAGINA);

	empaquetar_pagina(paquete, pagina);

	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

void empaquetar_pagina(t_paquete* paquete, t_pagina* pagina){
	if(pagina == NULL){
		pagina = malloc(sizeof(t_pagina));
		pagina->indice_tabla_de_pagina = -1;
		pagina->numero_pagina = -1;
	}
	agregar_valor_a_paquete(paquete, &(pagina->indice_tabla_de_pagina), sizeof(int));
	agregar_valor_a_paquete(paquete, &(pagina->numero_pagina), sizeof(int));
}

t_pagina* recibir_pagina(int socket_cliente){
	int desplazamiento = 0;
	int size;
	void * buffer;

	buffer = recibir_buffer(&size, socket_cliente);
	t_pagina* pagina = deserializar_pagina(&desplazamiento, buffer);

	free(buffer);
	return pagina;
}

t_pagina* deserializar_pagina(int* desplazamiento, void* buffer){
	t_pagina* pagina = malloc(sizeof(t_pagina));

	memcpy(&(pagina->indice_tabla_de_pagina), buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento) += sizeof(int);

	memcpy(&(pagina->numero_pagina), buffer + (*desplazamiento), sizeof(int));
	(*desplazamiento) += sizeof(int);

	return pagina;
}

void enviar_indices_tabla_paginas(t_list* indices, int socket_cliente) {
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(OKI_ESTRUCTURAS);
	
	empaquetar_strings(indices, paquete);
	
	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

void enviar_configuracion_memoria(int tamanio_pagina, int entradas, int cpu_fd) {
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(HANDSHAKE);

	agregar_valor_a_paquete(paquete, &tamanio_pagina, sizeof(int));
	agregar_valor_a_paquete(paquete, &entradas, sizeof(int));

	enviar_paquete(paquete, cpu_fd);
	eliminar_paquete(paquete);
}

t_pagina_config* recibir_configuracion_memoria(int socket_memoria) {
	int desplazamiento = 0;
	int size;
	void * buffer;
	
	t_pagina_config* pagina_config = malloc(sizeof(t_pagina_config));

	buffer = recibir_buffer(&size, socket_memoria);

	memcpy(&pagina_config->tamanio_pagina, buffer + desplazamiento, sizeof(int));

	desplazamiento += sizeof(int);

	memcpy(&pagina_config->cantidad_entradas, buffer + desplazamiento, sizeof(int));
	

	free(buffer);
	return pagina_config;
}


t_list* recibir_indices_tabla_paginas(int socket_cliente){
	int desplazamiento = 0;
	int size;
	void * buffer;
	t_list* lista_indices;

	buffer = recibir_buffer(&size, socket_cliente);

	lista_indices = deserializar_paquete_mensaje(&desplazamiento, buffer);
	
	free(buffer);
	return lista_indices;
}

/* SEGMENTOS */

void empaquetar_strings(t_list* strings, t_paquete* paquete){
	int cantidad_strings = list_size(strings);
	agregar_valor_a_paquete(paquete, &(cantidad_strings), sizeof(int));
	for (int i = 0; i < cantidad_strings; i++)
	{
		char *string = list_get(strings, i);
		agregar_a_paquete_con_header(paquete, string, strlen(string) + 1);
	}
}

void* deserializar_segmento(void* buffer, int* desplazamiento)
{
	t_segmento* nuevo_segmento = malloc(sizeof(t_segmento)); 

	memcpy(&(nuevo_segmento->nro_segmento), buffer + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	memcpy(&(nuevo_segmento->tamanio_segmento), buffer + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	memcpy(&(nuevo_segmento->indice_tabla_paginas), buffer + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	return nuevo_segmento;
}

t_list* deserializar_tabla_segmentos(int* desplazamiento, void* buffer){
	int cantidad_segmentos;
	t_list* tabla_segmentos = list_create();

	memcpy(&(cantidad_segmentos), buffer + (*desplazamiento), sizeof(int));
	*desplazamiento += sizeof(int);

	for(int i = 0; i < cantidad_segmentos; i++){
		t_segmento* nuevo_segmento = deserializar_segmento(buffer, desplazamiento);
		list_add(tabla_segmentos, nuevo_segmento);
	}

	return tabla_segmentos;
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
	agregar_valor_a_paquete(paquete, &(pcb->pid), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(pcb->program_counter), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(pcb->estado), sizeof(estado_proceso));
	agregar_valor_a_paquete(paquete, &(pcb->socket_consola), sizeof(int));
	agregar_valor_a_paquete(paquete, &(pcb->interrupcion), sizeof(bool));
	agregar_valor_a_paquete(paquete, &(pcb->con_desalojo), sizeof(bool));
	agregar_valor_a_paquete(paquete, &(pcb->page_fault), sizeof(bool));
	agregar_valor_a_paquete(paquete, &(pcb->segmentation_fault), sizeof(bool));
	empaquetar_registros(pcb->registros, paquete);
	empaquetar_pagina(paquete, pcb->pagina_fault);
	empaquetar_instrucciones(pcb->instrucciones, paquete);
	empaquetar_tabla_segmentos(pcb->tabla_de_segmentos, paquete);
}

void empaquetar_tabla_segmentos(t_list* tabla,t_paquete* paquete){

	int cant_segmentos = list_size(tabla);
	agregar_valor_a_paquete(paquete, &(cant_segmentos), sizeof(int));

	for(int i = 0; i < cant_segmentos; i++){
		t_segmento* segmento = list_get(tabla, i);
		agregar_valor_a_paquete(paquete, &(segmento->nro_segmento), sizeof(u_int32_t));
		agregar_valor_a_paquete(paquete, &(segmento->tamanio_segmento), sizeof(u_int32_t));
		agregar_valor_a_paquete(paquete, &(segmento->indice_tabla_paginas), sizeof(u_int32_t));
	}
	
}

void empaquetar_registros(registros_de_proposito_general registros, t_paquete* paquete){
	agregar_valor_a_paquete(paquete, &(registros.ax), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(registros.bx), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(registros.cx), sizeof(u_int32_t));
	agregar_valor_a_paquete(paquete, &(registros.dx), sizeof(u_int32_t));
}

void enviar_pcb_memoria(t_pcb_memoria* pcb, int socket_cliente) {
	t_paquete *paquete = new_paquete_con_codigo_de_operacion(ESTRUCTURAS);
	
	agregar_valor_a_paquete(paquete, &(pcb->pid), sizeof(int));
	empaquetar_strings(pcb->segmentos, paquete);
	
	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}

t_pcb* recibir_pcb(int socket_cliente){
	int desplazamiento = 0;
	int size;
	void * buffer;
	t_list* lista_instrucciones;
	t_list* tabla_segmentos;
	t_pagina *pagina;

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

	memcpy(&(nueva_pcb->page_fault), buffer + desplazamiento, sizeof(bool));
	desplazamiento += sizeof(bool);

	memcpy(&(nueva_pcb->segmentation_fault), buffer + desplazamiento, sizeof(bool));
	desplazamiento += sizeof(bool);

	memcpy(&(nueva_pcb->registros.ax), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.bx), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.cx), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);

	memcpy(&(nueva_pcb->registros.dx), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(u_int32_t);
	
	pagina = deserializar_pagina(&desplazamiento,buffer);
	lista_instrucciones = deserializar_instrucciones(&desplazamiento, buffer);
	tabla_segmentos = deserializar_tabla_segmentos(&desplazamiento, buffer);
	nueva_pcb->instrucciones = lista_instrucciones;
	nueva_pcb->tabla_de_segmentos = tabla_segmentos;
	nueva_pcb->pagina_fault = pagina;

	free(buffer);
	return nueva_pcb;
}

t_pcb_memoria* recibir_pcb_memoria(int socket_cliente) {
	int desplazamiento = 0;
	int size;
	void * buffer;

	buffer = recibir_buffer(&size, socket_cliente);

	t_pcb_memoria* nueva_pcb = malloc(sizeof(t_pcb_memoria)); 

	memcpy(&(nueva_pcb->pid), buffer + desplazamiento, sizeof(u_int32_t));
	desplazamiento += sizeof(int);

	nueva_pcb->segmentos = deserializar_paquete_mensaje(&desplazamiento, buffer);

	free(buffer);
	return nueva_pcb;
}

void enviar_pedido_de_escritura(int dir_fisica, int fd, int valor) {
	cod_mensaje cod_msj_test = ESCRIBIR;
	enviar_datos(fd, &cod_msj_test,sizeof(cod_msj_test));
	enviar_datos(fd, &dir_fisica,sizeof(dir_fisica));
	enviar_datos(fd, &valor,sizeof(valor));
}
