#include <esquema.h>

void crear_tablas_de_pagina(t_pcb_memoria* pcb) {
	int i = 0;
	int j = 0;
	int indice_base = list_size(lista_de_tablas_de_paginas);
	int cantidad_de_segmentos = list_size(pcb->segmentos);
	int cantidad_de_paginas = memoria_config->entradas_por_tabla;
	t_cursor* cursor = cursor_create(pcb->pid, indice_base, cantidad_de_segmentos);
	for(i = indice_base; i < indice_base + cantidad_de_segmentos; i++) {
		t_tabla_de_paginas* tabla_de_paginas = tabla_de_paginas_create(i, pcb->pid);
		for(j = 0; j < cantidad_de_paginas; j++) {
			t_entrada_tp* nueva_entrada_pagina = entrada_tp_create(pcb->pid);	
			list_add(tabla_de_paginas->entradas, nueva_entrada_pagina);
		}
		list_add(lista_de_tablas_de_paginas, tabla_de_paginas);
	}
	list_add(cursores, cursor);
}

t_cursor* cursor_create(int pid, int indice_tabla, int cantidad_segmentos){
	t_cursor* cursor = malloc(sizeof(t_cursor));
	cursor->pid = pid;
	cursor->indice_tabla_de_paginas = indice_tabla;
	cursor->entrada_tp = 0;
	cursor->cantidad_tablas = cantidad_segmentos;
	return cursor;
}

t_entrada_tp* entrada_tp_create(int pid){
	t_entrada_tp* nueva_entrada_pagina = malloc(sizeof(t_entrada_tp));
	nueva_entrada_pagina->marco = -1; 
	nueva_entrada_pagina->presencia = false;
	nueva_entrada_pagina->uso = false;
	nueva_entrada_pagina->modificado = false;
	nueva_entrada_pagina->posicion_swap = obtener_posicion_libre_swap(); // obtener posicion libre swap
	ocupar_posicion_swap(pid, nueva_entrada_pagina->posicion_swap);
	return nueva_entrada_pagina;
}

t_tabla_de_paginas* tabla_de_paginas_create(int indice, int pid){
	t_tabla_de_paginas* tabla_de_paginas = malloc(sizeof(t_tabla_de_paginas));
	tabla_de_paginas->entradas = list_create();
	tabla_de_paginas->indice_tabla_de_pagina = indice;
	tabla_de_paginas->pid = pid;
	return tabla_de_paginas;
}

int cantidad_de_marcos_en_memoria_proceso(int pid){
	t_list* marcos_por_proceso = obtener_marcos_memoria_por_pid(pid);
	int cantidad = list_size(marcos_por_proceso);
	list_destroy(marcos_por_proceso);
	return cantidad;
}

void rajar_pagina(t_entrada_tp* entrada_a_rajar){
	if(entrada_a_rajar->modificado){
		escribir_pagina_en_swap(entrada_a_rajar);
		entrada_a_rajar->modificado = 0;
	}
	liberar_marco_memoria(entrada_a_rajar);
	entrada_a_rajar->presencia = 0;
	log_debug(memoria_logger, "Victima rajada.");
}

// void cargar_pagina(t_pagina* pagina, t_marco* marco, int pid){
// 	t_entrada_tp* entrada_a_cargar = obtener_entrada_tp(pagina);
// 	poner_pagina_en_memoria_principal(pagina, marco, pid);
// 	entrada_a_cargar->presencia = true;
// 	entrada_a_cargar->marco = marco->numero_marco;
// 	entrada_a_cargar->uso = true;
// 	marco->pid = pid;
// }

void actualizar_cursor(int pid){
	t_cursor* cursor = obtener_cursor_por_pid(pid);
	t_tabla_de_paginas* primer_tabla = obtener_primer_tabla_pid(pid);
	int indice_base = primer_tabla->indice_tabla_de_pagina;
	int indice_limit = indice_base + cursor->cantidad_tablas - 1;
	if(cursor->entrada_tp == memoria_config->entradas_por_tabla - 1){
		if(cursor->indice_tabla_de_paginas == indice_limit){
			cursor->indice_tabla_de_paginas = primer_tabla->indice_tabla_de_pagina;
		}
		else {
			cursor->indice_tabla_de_paginas += 1;
		}
		cursor->entrada_tp = 0;
	}
	else {
		cursor->entrada_tp += 1;
	}
}

void armar_contexto(){
	int pid = 1;
	int contador = 6;
	int contador_marcos = 0;
	for(int i = 0; i < 4; i++){
		t_tabla_de_paginas* tabla = tabla_de_paginas_create(i, pid);
		for(int j = 0; j < memoria_config->entradas_por_tabla; j++){
			t_entrada_tp* nueva = entrada_tp_create(pid);
			nueva->marco = contador_marcos;
			contador_marcos++;
			if(contador > 0){
				nueva->presencia = 1;
				nueva->uso = 0;
				nueva->modificado = 1;
				contador--;
			}
			list_add(tabla->entradas, nueva);
		}
		list_add(lista_de_tablas_de_paginas, tabla);
	}
	t_cursor* cursor = cursor_create(pid, 0, 4);

	list_add(cursores, cursor);

	t_tabla_de_paginas* tabla_victima = list_get(lista_de_tablas_de_paginas, 0);
	t_entrada_tp* entrada_victima = list_get(tabla_victima->entradas, 0);
	
	entrada_victima->uso = 0;
	entrada_victima->marco = 1234;
	entrada_victima->modificado = 0;
	entrada_victima->presencia = 1;

	puts("Contexto creado");

}

void test_clock() {
	armar_contexto();

	t_entrada_tp* entrada_obtenida = obtener_victima_a_reemplazar(1);
	t_entrada_tp* esperada = entrada_tp_create(1);	
	esperada->marco = 1234;

	log_debug(memoria_logger, "entrada esperada --> marco: %d", esperada->marco);
	log_debug(memoria_logger, "entrada obtenida --> marco: %d", entrada_obtenida->marco);
}

t_entrada_tp* obtener_victima_a_reemplazar(int pid){
	t_tabla_de_paginas* primer_tabla = obtener_primer_tabla_pid(pid);
	t_cursor* cursor = obtener_cursor_por_pid(pid);
	bool hay_victima = false;
	t_entrada_tp* victima;
	int numero_vuelta = 1;
	
	while(!hay_victima){
		switch (memoria_config->algoritmo_reemplazo)
		{
		case CLOCK:
			victima = ejecutar_clock(cursor, &hay_victima);
			break;
		
		case CLOCK_M:
			victima = ejecutar_clock_m(cursor, &hay_victima, numero_vuelta);
			break;
		default:
			error_show("algoritmo desconocido");
			break;
		}
		if(!hay_victima){
			cursor->indice_tabla_de_paginas = primer_tabla->indice_tabla_de_pagina;
			cursor->entrada_tp = 0;
			numero_vuelta++;
		}
	}
	actualizar_cursor(pid);
	return victima;
}

t_entrada_tp* ejecutar_clock(t_cursor* cursor, bool* hay_victima){
	t_entrada_tp* victima;
	int indice_base_tabla = cursor->indice_tabla_de_paginas;
	int offset_tabla = cursor->cantidad_tablas;
	int indice_base_entrada = cursor->entrada_tp;
	int offset_entrada = memoria_config->entradas_por_tabla;
	log_debug(memoria_logger, "Comenzando Iteracion");
	for(int i = indice_base_tabla; i < (indice_base_tabla + offset_tabla); i++){
		log_debug(memoria_logger, "Tabla: %d", i);
		t_tabla_de_paginas* tabla_aux = list_get(lista_de_tablas_de_paginas, i);
		for(int j = indice_base_entrada; j < indice_base_entrada + offset_entrada; j++){
			t_entrada_tp* entrada_aux = list_get(tabla_aux->entradas, j);
			if(!entrada_aux->presencia) continue;
			log_debug(memoria_logger, "Entrada: %d  USO --> (%d)", j, entrada_aux->uso);
			if(!entrada_aux->uso){
				log_debug(memoria_logger, "Encontre víctima! --> entrada %d de tabla %d con marco %d", j, i, entrada_aux->marco);
				*hay_victima = true;
				victima = entrada_aux;
				cursor->indice_tabla_de_paginas = i;
				cursor->entrada_tp = j;
				break;
			}
			entrada_aux->uso = false;
		}
		if(*hay_victima) break;
	}
	return victima;
}

t_entrada_tp* ejecutar_clock_m(t_cursor* cursor, bool* hay_victima, int numero_vuelta){
	t_entrada_tp* victima;
	int indice_base_tabla = cursor->indice_tabla_de_paginas;
	int offset_tabla = cursor->cantidad_tablas;
	int indice_base_entrada = cursor->entrada_tp;
	int offset_entrada = memoria_config->entradas_por_tabla;
	log_debug(memoria_logger, "Comenzando Iteracion");
	for(int i = indice_base_tabla; i < (indice_base_tabla + offset_tabla); i++){
		log_debug(memoria_logger, "Tabla: %d", i);
		t_tabla_de_paginas* tabla_aux = list_get(lista_de_tablas_de_paginas, i);
		for(int j = indice_base_entrada; j < indice_base_entrada + offset_entrada; j++){
			t_entrada_tp* entrada_aux = list_get(tabla_aux->entradas, j);
			if(!entrada_aux->presencia) continue;
			log_debug(memoria_logger, "Entrada: %d  USO --> (%d) MODIFICADO --> (%d)", j, entrada_aux->uso, entrada_aux->modificado);
			if(cumple_clock_m_condicion_victima(entrada_aux, numero_vuelta)){
				log_debug(memoria_logger, "Encontre víctima! --> entrada %d de tabla %d con marco %d", j, i, entrada_aux->marco);
				*hay_victima = true;
				victima = entrada_aux;
				cursor->indice_tabla_de_paginas = i;
				cursor->entrada_tp = j;
				break;
			}
			if(numero_vuelta == 2) entrada_aux->uso = false;
		}
		if(*hay_victima) break;
	}
	return victima;
}

bool cumple_clock_m_condicion_victima(t_entrada_tp* entrada, int numero_vuelta){
	return !entrada->uso && ((!entrada->modificado && (numero_vuelta == 1 || numero_vuelta == 3)) || (entrada->modificado && numero_vuelta == 2));
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
		return cursor->pid == pid;
	}

	return list_find(cursores, (void*) by_pid);
}

t_tabla_de_paginas* obtener_primer_tabla_pid(int pid){
	bool by_pid(t_tabla_de_paginas* tabla){
		return tabla->pid == pid;
	}
	return list_find(lista_de_tablas_de_paginas, (void*) by_pid);	
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