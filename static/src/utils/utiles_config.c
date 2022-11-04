/*
 * utiles_config.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */
#include <utils/utiles_config.h>


//Levanta todos los datos que necesitamos del config
void* cargar_configuracion(char* path_archivo, void* (*configurar_modulo)(t_config*)) {
	t_config *config;
	void* modulo_config;

	char* config_path = strdup(path_archivo);
	config = config_create(config_path);
	if (!validar_configuracion(config)) {
		error_show("No se encontró el archivo de configuración.");
		free(config_path);
		config_destroy(config);
		exit(EXIT_FAILURE);
	}
	modulo_config = configurar_modulo(config);
	config_destroy(config);
	free(config_path);
	return modulo_config;
}

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

t_list* config_get_io_list(t_config* config){
	t_list* lista_dispositivos = list_create();
	char** array_dispositivos = config_get_array_value(config, "DISPOSITIVOS_IO");
	char** array_duraciones = config_get_array_value(config, "TIEMPOS_IO");
	for(int i = 0; i < string_array_size(array_dispositivos); i++){
		t_dispositivo* new_dispositivo = malloc(sizeof(t_dispositivo));
		t_queue* cola = queue_create();
		new_dispositivo->indice = i;
		new_dispositivo->nombre = strdup(array_dispositivos[i]);
		new_dispositivo->duracion = atoi(array_duraciones[i]);
		new_dispositivo->cola = cola;
		list_add(lista_dispositivos, new_dispositivo);
	}
	string_array_destroy(array_dispositivos);
	string_array_destroy(array_duraciones);
	return lista_dispositivos;
}

t_algoritmo config_get_algoritmo_enum(t_config* config){
	char* algoritmo_string = strdup(config_get_string_value(config,"ALGORITMO_PLANIFICACION"));
	t_algoritmo algoritmo = FIFO; // default por si hay errores, sacar en algun momento(?
	if(string_equals_ignore_case(algoritmo_string,"FIFO")){
		algoritmo = FIFO;
	}
	else if(string_equals_ignore_case(algoritmo_string,"RR")){
		algoritmo = RR;
	}
	else if(string_equals_ignore_case(algoritmo_string,"FEEDBACK")){
		algoritmo = FEEDBACK;
	}
	free(algoritmo_string);
	return algoritmo;
}
