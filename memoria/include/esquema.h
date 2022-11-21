#ifndef MEMORIA_INCLUDE_ESQUEMA_H_
#define MEMORIA_INCLUDE_ESQUEMA_H_

#include <memoria_utils.h>

typedef struct {
    int pid;
    int indice_tabla_de_paginas;
    int entrada_tp;
    int cantidad_tablas;
}t_cursor;

void crear_tablas_de_pagina(t_pcb_memoria*);
t_list* obtener_indices_tablas_de_pagina(t_pcb_memoria*);
t_cursor* cursor_create(int, int, int);
void actualizar_cursor(int);
t_tabla_de_paginas* tabla_de_paginas_create(int, int);
t_entrada_tp* entrada_tp_create(int);
t_tabla_de_paginas* obtener_primer_tabla_pid(int);
t_list* obtener_tablas_por_pid(int);
void rajar_pagina(t_entrada_tp*);
t_list* obtener_marcos_memoria_por_pid(int);
bool cursor_tabla_presente(t_tabla_de_paginas*);
t_tabla_de_paginas* obtener_tabla_con_cursor(int);
bool cumple_clock_m_condicion_victima(t_entrada_tp*, int);
t_entrada_tp* ejecutar_clock_m(t_cursor*, bool*, int);
int cantidad_de_marcos_en_memoria_proceso(int);
int cantidad_de_paginas_en_memoria_proceso(int);
t_entrada_tp* obtener_victima_a_reemplazar(int);
t_entrada_tp* ejecutar_clock(t_cursor*,  bool*);
void armar_contexto();
void test_clock();
t_cursor* obtener_cursor_por_pid(int);

#endif /* MEMORIA_INCLUDE_ESQUEMA_H_ */
