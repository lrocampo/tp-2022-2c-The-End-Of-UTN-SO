#include <swap.h>

void swap_init(){
	pthread_mutex_init(&memoria_swap_mutex, NULL);
    marcos_swap_init();
    swap_create();

}

void marcos_swap_init() {
    log_debug(memoria_logger, "Cargando marcos swap...");
	lista_de_marcos_swap = list_create();

	marcos_init(lista_de_marcos_swap, memoria_config->tamanio_swap, memoria_config->tamanio_pagina);
}

void swap_create(){
    int swap_fd = open(memoria_config->path_swap,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
    if(swap_fd == -1){
        error_show("Error abriendo Swap.");
        exit(EXIT_FAILURE);
    }   
    ftruncate(swap_fd,memoria_config->tamanio_swap);


    swap = mmap(NULL,memoria_config->tamanio_swap,PROT_WRITE|PROT_READ, MAP_FILE|MAP_SHARED,swap_fd,0);
	if (swap == MAP_FAILED)
		    {
		        close(swap_fd);
		        error_show("Error mmapping the file");
		    }
}

int obtener_posicion_libre_swap(){
    //t_marco* marco_buscado = list_find(lista_de_marcos_swap,  (void*) marco_libre);
    return obtener_marco_libre(lista_de_marcos_swap)->numero_marco * memoria_config->tamanio_pagina;//marco_buscado->numero_marco * memoria_config->tamanio_pagina;
}

void ocupar_posicion_swap(int pid, int posicion){
    t_marco* marco_buscado = list_get(lista_de_marcos_swap, posicion / memoria_config->tamanio_pagina);
    marco_buscado->pid = pid;
}

void escribir_pagina_en_swap(t_entrada_tp* entrada_pagina){
	log_debug(memoria_logger,"Escribiendo pagina en swap \n");
	int posicion = entrada_pagina->marco * memoria_config->tamanio_pagina;
    void* source = espacio_memoria + posicion;
    void* dest = swap + entrada_pagina->posicion_swap;
    escribir_en_memoria(dest, source);
    log_debug(memoria_logger,"Se escribio pagina en swap \n");
}

void poner_pagina_en_memoria_principal(t_entrada_tp* entrada_pagina){
	log_debug(memoria_logger,"Escribiendo pagina en memoria principal \n");
    int posicion = entrada_pagina->marco * memoria_config->tamanio_pagina;
	void* dest = espacio_memoria + posicion;
    void* source = swap + entrada_pagina->posicion_swap;
    escribir_en_memoria(dest, source);
	log_debug(memoria_logger,"Se escribio pagina en memoria principal \n");
}

void escribir_en_memoria(void* dest, void* source){
    ejecutar_espera(memoria_config->retardo_swap);
    pthread_mutex_lock(&memoria_swap_mutex);
	pthread_mutex_lock(&memoria_usuario_mutex);
	memcpy(dest, source, sizeof(memoria_config->tamanio_pagina));
	pthread_mutex_unlock(&memoria_usuario_mutex);
    pthread_mutex_unlock(&memoria_swap_mutex);
}

void* obtener_pagina_de_swap(t_entrada_tp* entrada_pagina){
    log_info(memoria_logger,"Obteniendo pagina de swap \n");
    void* pagina = NULL;
    ejecutar_espera(memoria_config->retardo_swap);
    pthread_mutex_lock(&memoria_swap_mutex);
    memcpy(pagina, swap + entrada_pagina->posicion_swap, memoria_config->tamanio_pagina);
    pthread_mutex_unlock(&memoria_swap_mutex);
	log_info(memoria_logger,"Se Obtuvo pagina en swap \n");
    return pagina;
}
