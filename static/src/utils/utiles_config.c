/*
 * utiles_config.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */
#include <utils/utiles_config.h>

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

t_algoritmo config_get_algoritmo_enum(t_config* config){
	char* algoritmo_string = strdup(config_get_string_value(config,"ALGORITMO_PLANIFICACION"));
	t_algoritmo algoritmo = FIFO; // default por si hay errores, sacar en algun momento(?
	if(strcmp(algoritmo_string,"FIFO") == 0){
		algoritmo = FIFO;
	}
	else if(strcmp(algoritmo_string,"RR") == 0){
		algoritmo = RR;
	}
	else if(strcmp(algoritmo_string,"FEEDBACK") == 0){
		algoritmo = FEEDBACK;
	}
	free(algoritmo_string);
	return algoritmo;
}

//Levanta todos los datos que necesitamos del config
void* cargar_configuracion(char* path_archivo, t_tipo_archivo tipo_archivo) {
	t_config *config;

	char* config_path = strdup(path_archivo);
	config = config_create(config_path);
	if (!validar_configuracion(config)) {
		error_show("No se encontró el archivo de configuración.");
		free(config_path);
		config_destroy(config);
		exit(EXIT_FAILURE);
	}

	switch (tipo_archivo) {
		case CONSOLA:
			t_consola_config* consola_config;
			consola_config = malloc(sizeof(t_consola_config));
			consola_config->ip = strdup(config_get_string_value(config, "IP"));
			consola_config->puerto = strdup(config_get_string_value(config, "PUERTO"));
			// TODO: Componer la lista de segmentos
			config_destroy(config);
			free(config_path);
			return consola_config;
		case KERNEL:
			t_kernel_config* kernel_config;
			kernel_config = malloc(sizeof(t_kernel_config));
			kernel_config->ip_cpu = strdup(config_get_string_value(config, "IP_CPU"));
			kernel_config->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
			kernel_config->puerto_escucha = strdup(config_get_string_value(config, "PUERTO_ESCUCHA"));
			kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config, "PUERTO_CPU_DISPATCH"));
			kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config, "PUERTO_CPU_INTERRUPT"));
			kernel_config->grado_multiprogramacion = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
			kernel_config->algoritmo = config_get_algoritmo_enum(config);

			// TODO: Componer la configuracion del resto

			config_destroy(config);
			free(config_path);
			return kernel_config;
		case CPU:
			t_cpu_config* cpu_config;
			cpu_config = malloc(sizeof(t_cpu_config));
			cpu_config->ip_cpu = strdup(config_get_string_value(config, "IP_CPU"));
			cpu_config->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
			cpu_config->puerto_escucha_dispatch = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"));
			cpu_config->puerto_escucha_interrupt = strdup(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"));
			
			config_destroy(config);
			free(config_path);
			return cpu_config;

		default:
			config_destroy(config);
			error_show("cargando configuracion tipo de archivo invalido");
			exit(EXIT_FAILURE);
	}
}