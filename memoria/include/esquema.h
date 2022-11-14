#ifndef MEMORIA_INCLUDE_ESQUEMA_H_
#define MEMORIA_INCLUDE_ESQUEMA_H_

#include <memoria_utils.h>

void crear_tablas_de_pagina(t_pcb_memoria*);
t_list* obtener_indices_tablas_de_pagina(t_pcb_memoria*);
bool pagina_presente(t_entrada_tp*);
t_list* obtener_tablas_por_pid(int);
int cantidad_de_paginas_en_memoria_proceso(int);

#endif /* MEMORIA_INCLUDE_ESQUEMA_H_ */
