#include <consola_parser.h>

 t_list* obtener_pseudocodigo(char* instructions_string) {
    t_list *pseudocodigo = list_create();
    char* param1 = NULL;
    char* param2 = NULL;
    char **palabras = NULL;
    char **instructions_array = string_split(instructions_string, "\n");
    int size = string_array_size(instructions_array);
    for(int i = 0; i < size; i++){    
        palabras = string_split(instructions_array[i], " ");
        cod_operacion cod_op = string_to_cod_op(palabras[0]);
        switch (cod_op) {
            case SET:
            case ADD:
            case MOV_IN:
            case MOV_OUT:
            case IO:
                param1 = strdup(palabras[1]);
                param2 = strdup(palabras[2]);
            case EXIT:
                list_add(pseudocodigo, (void*) new_instruccion(cod_op, param1, param2));
                break;
            default:
                error_show("Error al leer la instruccion: %d\n",i);
                exit(EXIT_FAILURE);
                break;
        }
        string_array_destroy(palabras);
        free(param1);
        free(param2);
        param1 = NULL;
        param2 = NULL;
    }
    string_array_destroy(instructions_array);
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
        estructura->parametro1 = strdup("");
        estructura->parametro2 = strdup("");
    }

    return estructura;
}