#ifndef MEMORIA_INCLUDE_SWAP_H_
#define MEMORIA_INCLUDE_SWAP_H_

#include <memoria_utils.h>
#include <sys/mman.h>
//#include <sys/stat.h>
//#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>

void swap_init();
void escribir_pagina_en_swap(t_entrada_tp*);
int obtener_posicion_libre_swap();
void* obtener_pagina_de_swap(t_entrada_tp* );
void cargar_pagina_en_memoria_principal(t_pagina*, t_marco*, int);
void ocupar_posicion_swap(int, int);
void swap_create();
void marcos_swap_init();

#endif /* MEMORIA_INCLUDE_SWAP_H_ */
