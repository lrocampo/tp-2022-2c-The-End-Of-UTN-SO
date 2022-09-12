/*
 * socket.h
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#ifndef INCLUDE_UTILS_SOCKET_H_
#define INCLUDE_UTILS_SOCKET_H_

#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<utils/comunicacion.h>

int crear_conexion( char* , char* );
void liberar_conexion(int socket_cliente);
int iniciar_servidor(void);
int esperar_cliente(int);

#endif /* INCLUDE_UTILS_SOCKET_H_ */
