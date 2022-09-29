#include <consola_parser.h>

 t_list* obtener_pseudocodigo(char* ruta) {
    t_list *pseudocodigo = list_create();
    char** param1, param2;
    char *instructions_string = leerArchivo(ruta);
    char **instructions_array = string_split(instructions_string, "\n");
    int indice = 0;

    while(instructions_array[indice] != NULL) {
        char **palabras = string_split(instructions_array[indice], " ");

        if(string_equals_ignore_case(palabras[0], "SET")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, new_instruccion(SET, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "ADD")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, new_instruccion(ADD, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "MOV_IN")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, new_instruccion(MOV_IN, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "MOV_OUT")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, new_instruccion(MOV_OUT, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "I/O")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, new_instruccion(IO, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "EXIT")) {
            param1 = NULL;
            param2 = NULL;
            list_add(pseudocodigo, new_instruccion(EXIT, param1, param2));
        }
        else {
            perror("Error al leer la instruccion");
            exit(1);
        }
        indice++;
        free(palabras);
    }

    return pseudocodigo;
}

 /// ** Manejo de arhchivo *** ///
char *leerArchivo (char *ruta)
{
	char pathInstrucciones[100];
	strcpy(pathInstrucciones, ruta);
	FILE *archivo = fopen(ruta, "r"); /// declaro el archivo y lo leo
	if (archivo == NULL)
    {
        perror("Error al abrir el archivo.\n");
    }

	//posiciono el puntero para obtener el total de elementos del archivo
	fseek(archivo,0,SEEK_END);
	int cantElementos = ftell(archivo);
	rewind(archivo);

	char *arrayDinamico = calloc(cantElementos+1,sizeof(char));
	int cantElementosLeidos = fread(arrayDinamico, sizeof(char),cantElementos,archivo);

	if(cantElementos != cantElementosLeidos )
	{
		perror("Error leyendo el archivo.\n") ;
	}

	fclose(archivo);
    // log_info(consola_logger,"\nSe ha leido el archivo de instrucciones correctamente.\n");
    return arrayDinamico;
}

instruccion* new_instruccion(cod_operacion operacion, char* parametro1, char* parametro2) {
    instruccion *estructura = malloc(sizeof(instruccion));
    estructura->operacion = operacion;
    estructura->parametro1 = strdup(parametro1);
    estructura->parametro2 = strdup(parametro2);
}