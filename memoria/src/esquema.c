#include <esquema.h>

void crear_tablas_de_pagina(t_pcb_memoria* pcb) {
	int i = 0;
	int j = 0;
	int cantidad_de_segmentos = list_size(pcb->segmentos);
	int cantidad_de_paginas = memoria_config->entradas_por_tabla;
	for(i = 0; i < cantidad_de_segmentos; i++) {
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
	}
}

int cantidad_de_paginas_en_memoria_proceso(int pid){
	int cantidad = 0;

	t_list* tablas_paginas_proceso = obtener_tablas_por_pid(pid);
	for(int i = 0; i < list_size(tablas_paginas_proceso); i++){
		t_tabla_de_paginas* tabla = list_get(tablas_paginas_proceso, i);
		t_list* entradas_presentes = list_filter(tabla->entradas, (void*) pagina_presente);
		cantidad += list_size(entradas_presentes);
		list_destroy(entradas_presentes);
	}
	list_destroy(tablas_paginas_proceso);
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

t_list* obtener_tablas_por_pid(int pid){
	bool by_pid(t_tabla_de_paginas* tabla){
		return tabla->pid == pid;
	}
	return list_filter(lista_de_tablas_de_paginas, (void*) by_pid);
}