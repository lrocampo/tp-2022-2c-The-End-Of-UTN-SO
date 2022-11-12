#include <swap.h>

void swap_init(){
	pthread_mutex_init(&memoria_swap_mutex, NULL);
    marcos_swap_init();
    swap_create();

}

void marcos_swap_init() {
    log_debug(memoria_logger, "Cargando marcos swap...");
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
    t_marco* marco = list_find(lista_de_marcos_swap, marco_libre);
    return marco->numero_marco * memoria_config->tamanio_pagina;
}

void escribir_en_swap(t_pagina* pagina){
	log_info(memoria_logger,"Escribiendo pagina en swap \n");
    ejecutar_espera(memoria_config->retardo_swap);
    pthread_mutex_lock(&memoria_swap_mutex);
    pthread_mutex_lock(&memoria_usuario_mutex);
    memcpy(swap + pagina->posicion_swap, espacio_memoria + (pagina->marco * memoria_config->tamanio_pagina), memoria_config->tamanio_pagina);
    pthread_mutex_unlock(&memoria_usuario_mutex);
    pthread_mutex_unlock(&memoria_swap_mutex);
	log_info(memoria_logger,"Se escribio pagina en swap \n");
    
    // t_pagina* pagina = malloc(sizeof(t_pagina));
	// log_info(memoria_logger,"Entrando a swap_write \n");
    // // pagina->posicion_swap = 2048;
    // // int valor = 20;
    // // int valor_swap;
    // memcpy(swap + pagina->posicion_swap, &valor, memoria_config->tamanio_pagina);
    // memcpy(&valor_swap, swap + pagina->posicion_swap, memoria_config->tamanio_pagina);
    // printf("Valor escrito:%d\n", valor_swap);
    // // usleep(config_valores_memoria.retardo_swap * 1000);
	// // pthread_mutex_lock(&mutex_memoria_usuario);
	// // pthread_mutex_lock(&mutex_archivo_swap);
	// //memcpy(archivo_swap+get_marco(pag->indice),memoria_usuario+get_marco(pag->marco),config_valores_memoria.tam_pagina);
	// // pthread_mutex_unlock(&mutex_memoria_usuario);
	// // pthread_mutex_unlock(&mutex_archivo_swap);
	// log_info(memoria_logger,"Se escribio pagina en swap \n");
}
