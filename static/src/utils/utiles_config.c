/*
 * utiles_config.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */
#include <utils/utiles_config.h>

// Levanta todos los datos que necesitamos del config
void *cargar_configuracion(char *path_archivo, void *(*configurar_modulo)(t_config *))
{
	t_config *config;
	void *modulo_config;

	char *config_path = strdup(path_archivo);
	config = config_create(config_path);
	if (!validar_configuracion(config))
	{
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

bool validar_configuracion(t_config *config)
{
	return (config_keys_amount(config) > 0);
}

t_algoritmo config_get_algoritmo_enum(t_config *config, char *algoritmo_type)
{
	char *algoritmo_string = strdup(config_get_string_value(config, algoritmo_type));
	t_algoritmo algoritmo = FIFO; // default por si hay errores, sacar en algun momento(?
	if (string_equals_ignore_case(algoritmo_string, "FIFO"))
	{
		algoritmo = FIFO;
	}
	else if (string_equals_ignore_case(algoritmo_string, "RR"))
	{
		algoritmo = RR;
	}
	else if (string_equals_ignore_case(algoritmo_string, "FEEDBACK"))
	{
		algoritmo = FEEDBACK;
	}
	else if (string_equals_ignore_case(algoritmo_string, "LRU"))
	{
		algoritmo = LRU;
	}
	else if (string_equals_ignore_case(algoritmo_string, "CLOCK"))
	{
		algoritmo = CLOCK;
	}
	else if (string_equals_ignore_case(algoritmo_string, "CLOCK-M"))
	{
		algoritmo = CLOCK_M;
	}
	free(algoritmo_string);
	return algoritmo;
}
