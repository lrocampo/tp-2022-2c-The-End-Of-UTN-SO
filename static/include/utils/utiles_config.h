#ifndef INCLUDE_UTILS_UTILES_CONFIG_H_
#define INCLUDE_UTILS_UTILES_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/error.h>
#include <string.h>
#include <utils/contexto.h>

void *cargar_configuracion(char *, void *(*)(t_config *));
bool validar_configuracion(t_config *);
t_algoritmo config_get_algoritmo_enum(t_config *, char *);

#endif /* INCLUDE_UTILS_UTILES_CONFIG_H_ */
