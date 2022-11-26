#ifndef INCLUDE_UTILS_SOCKET_H_
#define INCLUDE_UTILS_SOCKET_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/error.h>

int crear_conexion(char *, char *);
void liberar_conexion(int);
int iniciar_servidor(char *, char *);
int esperar_cliente(int);
void liberar_conexion(int);

#endif /* INCLUDE_UTILS_SOCKET_H_ */
