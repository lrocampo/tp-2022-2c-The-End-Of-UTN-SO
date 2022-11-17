#ifndef MEMORIA_INCLUDE_ESQUEMA_H_
#define MEMORIA_INCLUDE_ESQUEMA_H_

#include <memoria_utils.h>

typedef struct {
    int pid;
    int indice_tabla_de_paginas;
    int entrada_tp;
}t_cursor;

void crear_tablas_de_pagina(t_pcb_memoria*);
t_list* obtener_indices_tablas_de_pagina(t_pcb_memoria*);
bool pagina_presente(t_entrada_tp*);
t_list* obtener_tablas_por_pid(int);
t_list* obtener_marcos_memoria_por_pid(int);
bool cursor_tabla_presente(t_tabla_de_paginas*);
t_tabla_de_paginas* obtener_tabla_con_cursor(int);
int cantidad_de_marcos_en_memoria_proceso(int);
int cantidad_de_paginas_en_memoria_proceso(int);

#endif /* MEMORIA_INCLUDE_ESQUEMA_H_ */
