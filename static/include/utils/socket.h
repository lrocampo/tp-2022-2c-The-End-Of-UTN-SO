#ifndef STATIC_SOCKET_H_
#define STATIC_SOCKET_H_

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

#define IP "127.0.0.1"
#define PUERTO "3000"

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




int crear_conexion(char*, char*);
void enviar_mensaje(char*, int);
int iniciar_servidor(void);
int esperar_cliente(int);
void recibir_mensaje(int);
void* recibir_buffer(int*, int);
void* serializar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
void liberar_conexion(int);
int recibir_operacion(int);






#endif
