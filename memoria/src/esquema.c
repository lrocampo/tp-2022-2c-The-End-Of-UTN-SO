#include <esquema.h>

void crear_tablas_de_pagina(t_pcb_memoria* pcb) {
	int i = 0;
	int j = 0;
	int indice_base = list_size(lista_de_tablas_de_paginas) - 1;
	int cantidad_de_segmentos = list_size(pcb->segmentos);
	int cantidad_de_paginas = memoria_config->entradas_por_tabla;
	t_cursor* cursor = cursor_create(pcb->pid, indice_base);
	for(i = indice_base; i < indice_base + cantidad_de_segmentos; i++) {
		t_tabla_de_paginas* tabla_de_paginas = malloc(sizeof(t_tabla_de_paginas));
		tabla_de_paginas->entradas = list_create();
		tabla_de_paginas->indice_tabla_de_pagina = i;
		tabla_de_paginas->pid = pcb->pid;
		for(j = 0; j < cantidad_de_paginas; j++) {
			t_entrada_tp* nueva_entrada_pagina = malloc(sizeof(t_entrada_tp));
			nueva_entrada_pagina->marco = -1; 
			nueva_entrada_pagina->presencia = false;
			nueva_entrada_pagina->uso = false;
			nueva_entrada_pagina->modificado = false;
			nueva_entrada_pagina->posicion_swap = obtener_posicion_libre_swap(); // obtener posicion libre swap
			ocupar_posicion_swap(pcb->pid, nueva_entrada_pagina->posicion_swap);
			list_add(tabla_de_paginas->entradas, nueva_entrada_pagina);
		}
		list_add(lista_de_tablas_de_paginas, tabla_de_paginas);
		list_add(cursores, cursor);
	}
}

t_cursor* cursor_create(int pid, int indice_tabla){
	t_cursor* cursor = malloc(sizeof(t_cursor));
	cursor->pid = pid;
	cursor->indice_tabla_de_paginas = indice_tabla;
	cursor->entrada_tp = 0;
	return cursor;
}

int cantidad_de_marcos_en_memoria_proceso(int pid){
	t_list* marcos_por_proceso = obtener_marcos_memoria_por_pid(pid);
	int cantidad = list_size(marcos_por_proceso);
	list_destroy(marcos_por_proceso);
	return cantidad;
}

bool pagina_presente(t_entrada_tp* entrada){
	return entrada->presencia;
}

t_list* obtener_indices_tablas_de_pagina(t_pcb_memoria* pcb){
	
	t_list* tablas_buscadas;
	t_list* indices;

	tablas_buscadas = obtener_tablas_por_pid(pcb->pid);

	char* to_idx(t_tabla_de_paginas* tabla){
		return string_itoa(tabla->indice_tabla_de_pagina);
	}	

	log_debug(memoria_logger, "cantidad de tablas de pid: %d : %d", pcb->pid, list_size(tablas_buscadas));

	indices = list_map(tablas_buscadas, (void*) to_idx);
	list_destroy(tablas_buscadas);

	return indices;
}

void ejecutar_algoritmo(int pid){
	t_list* tablas_proceso = obtener_tablas_por_pid(pid);
	t_tabla_de_paginas* primer_tabla = list_get(tablas_proceso, 0);
	t_cursor* cursor = obtener_cursor_por_pid(pid);
	int indice_base_tabla = cursor->indice_tabla_de_paginas;
	int offset_tabla = list_size(tablas_proceso);
	int indice_base_entrada = cursor->entrada_tp;
	int offset_entrada = memoria_config->entradas_por_tabla;
	bool hay_victima = false;
	t_entrada_tp* victima;
	
	while(!hay_victima){
		for(int i = indice_base_tabla; i < indice_base_tabla + offset_tabla; i++){
		t_tabla_de_paginas* tabla_aux = list_get(lista_de_tablas_de_paginas, i);
			for(int j = indice_base_entrada; j < indice_base_entrada + offset_entrada; j++){
				t_entrada_tp* entrada_aux = list_get(tabla_aux->entradas, j);
				if(!entrada_aux->presencia){
					continue;
				}
				if(!entrada_aux->uso){
					hay_victima = true;
					victima = entrada_aux;
					cursor->indice_tabla_de_paginas = i;
					cursor->entrada_tp = j;
					break;
				}
				entrada_aux->uso = false;
			}
		}
		indice_base_tabla = primer_tabla->indice_tabla_de_pagina;
		indice_base_entrada = 0;
	}
	list_destroy(tablas_proceso);
}

t_list* obtener_tablas_por_pid(int pid){
	bool by_pid(t_tabla_de_paginas* tabla){
		return tabla->pid == pid;
	}
	return list_filter(lista_de_tablas_de_paginas, (void*) by_pid);
}

t_list* obtener_marcos_memoria_por_pid(int pid){

	bool by_pid(t_marco* marco){
		return marco->pid == pid;
	}
	
	return list_filter(lista_de_marcos, (void*) by_pid);
}

t_cursor* obtener_cursor_por_pid(int pid){
	bool by_pid(t_cursor* cursor){
		return marco->pid == pid;
	}

	return list_find(cursores, (void*) by_pid);
}