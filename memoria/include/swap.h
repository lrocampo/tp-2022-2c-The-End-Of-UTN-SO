#ifndef MEMORIA_INCLUDE_SWAP_H_
#define MEMORIA_INCLUDE_SWAP_H_

#include <memoria_utils.h>
#include <sys/mman.h>
//#include <sys/stat.h>
//#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

extern void* swap;

extern void* espacio_memoria;
extern pthread_mutex_t memoria_swap_mutex;
extern pthread_mutex_t memoria_usuario_mutex;


void swap_init();
void escribir_pagina_en_swap(t_entrada_tp*);
int obtener_posicion_libre_swap();
void* obtener_pagina_de_swap(t_entrada_tp* );
void ocupar_posicion_swap(int, int);
void swap_create();
void marcos_swap_init();

#endif /* MEMORIA_INCLUDE_SWAP_H_ */
