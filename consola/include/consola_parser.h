#ifndef CONSOLA_PARSER_INCLUDE_CONSOLA_H_
#define CONSOLA_PARSER_INCLUDE_CONSOLA_H_

#include <utils/utiles_config.h>
#include <utils/logger.h>
#include <utils/contexto.h>
#include <stdio.h>
#include <commons/string.h>
#include <commons/error.h>


t_list* obtener_pseudocodigo(char*);
char *leer_archivo_pseudocodigo (char*);
instruccion* instruccion_create(cod_operacion, char*, char*);

#endif /* CONSOLA_PARSER_INCLUDE_CONSOLA_H_ */







