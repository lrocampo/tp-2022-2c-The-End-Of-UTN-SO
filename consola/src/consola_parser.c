#include <consola_parser.h>

 t_list* obtener_pseudocodigo(char* instructions_string) {
    t_list *pseudocodigo = list_create();
    char* param1;
    char* param2;
    char **instructions_array = string_split(instructions_string, "\n");
    int size = string_array_size(instructions_array);
    for(int i = 0; i < size; i++){    
        char **palabras = string_split(instructions_array[i], " ");

        if(string_equals_ignore_case(palabras[0], "SET")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, (void*)new_instruccion(SET, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "ADD")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, (void*)new_instruccion(ADD, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "MOV_IN")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, (void*)new_instruccion(MOV_IN, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "MOV_OUT")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, (void*)new_instruccion(MOV_OUT, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "I/O")) {
            param1 = strdup(palabras[1]);
            param2 = strdup(palabras[2]);
            list_add(pseudocodigo, (void*)new_instruccion(IO, param1, param2));
        }
        else if(string_equals_ignore_case(palabras[0], "EXIT")) {
            param1 = NULL;
            param2 = NULL;
            list_add(pseudocodigo, (void*)new_instruccion(EXIT, param1, param2));
        }
        else {
            error_show("Error al leer la instruccion: %d\n",i);
            exit(EXIT_FAILURE);
        }
        free(palabras);
    }
    return pseudocodigo;
}

 /// ** Manejo de arhchivo *** ///
char *leer_archivo_pseudocodigo (char *ruta)
{
	char pathInstrucciones[100];
	strcpy(pathInstrucciones, ruta);
	FILE *archivo = fopen(pathInstrucciones, "r"); /// declaro el archivo y lo leo
	if (archivo == NULL)
    {
        error_show("Error al abrir el archivo.\n");
        exit(EXIT_FAILURE);
    }

	//posiciono el puntero para obtener el total de elementos del archivo
	fseek(archivo,0,SEEK_END);
	int cantElementos = ftell(archivo);
	rewind(archivo);

	char *arrayDinamico = calloc(cantElementos + 1,sizeof(char));
	int cantElementosLeidos = fread(arrayDinamico, sizeof(char),cantElementos,archivo);

	if(cantElementos != cantElementosLeidos )
	{
		error_show("Error leyendo el archivo.\n");
        exit(EXIT_FAILURE);
	}

	fclose(archivo);
    return arrayDinamico;
}

instruccion* new_instruccion(cod_operacion operacion, char* parametro1, char* parametro2) {
    instruccion *estructura = malloc(sizeof(instruccion));
    estructura->operacion = operacion;
    if(parametro1 != NULL && parametro2 != NULL){
        estructura->parametro1 = strdup(parametro1);
        estructura->parametro2 = strdup(parametro2);
    } else
    {
        estructura->parametro1 = "";
        estructura->parametro2 = "";
    }

    return estructura;
}