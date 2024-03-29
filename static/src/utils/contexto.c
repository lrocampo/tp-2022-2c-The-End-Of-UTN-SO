/*
 * contexto.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#include <utils/contexto.h>

/* Creacion */

t_dispositivo *dispositivo_io_create(int indice, char **array_dispositivos, char **array_duraciones)
{
    t_dispositivo *new_dispositivo = malloc(sizeof(t_dispositivo));
    t_queue *cola = queue_create();
    new_dispositivo->indice = indice;
    new_dispositivo->nombre = strdup(array_dispositivos[indice]);
    new_dispositivo->duracion = atoi(array_duraciones[indice]);
    new_dispositivo->cola = cola;
    return new_dispositivo;
}

instruccion *instruccion_create(cod_operacion operacion, char *parametro1, char *parametro2)
{
    instruccion *estructura = malloc(sizeof(instruccion));
    estructura->operacion = operacion;
    if (parametro1 != NULL && parametro2 != NULL)
    {
        estructura->parametro1 = strdup(parametro1);
        estructura->parametro2 = strdup(parametro2);
    }
    else
    {
        estructura->parametro1 = strdup("");
        estructura->parametro2 = strdup("");
    }

    return estructura;
}

t_segmento *segmento_create(int nro, int indice_tabla, int tamanio)
{
    t_segmento *segmento = malloc(sizeof(t_segmento));
    segmento->nro_segmento = nro;
    segmento->tamanio_segmento = tamanio;
    segmento->indice_tabla_paginas = indice_tabla;
    return segmento;
}

t_pcb *pcb_create(t_proceso *proceso, int pid, int socket)
{
    t_pcb *pcb = malloc(sizeof(t_pcb));

    pcb->estado = NEW;
    pcb->instrucciones = proceso->instrucciones;
    pcb->pid = pid;
    pcb->socket_consola = socket;
    pcb->program_counter = 0;
    pcb->page_fault = false;
    pcb->segmentation_fault = false;
    pcb->interrupcion = false;
    pcb->registros.ax = 0;
    pcb->registros.bx = 0;
    pcb->registros.cx = 0;
    pcb->registros.dx = 0;
    pcb->pagina_fault = NULL;
    pcb->tabla_de_segmentos = NULL;
    pcb->con_desalojo = false;
    pcb->tamanio_segmentos = proceso->segmentos;

    return pcb;
}

t_pagina *pagina_create(int indice_tabla_paginas, int numero_pagina)
{
    t_pagina *pagina = malloc(sizeof(t_pagina));

    pagina->indice_tabla_de_pagina = indice_tabla_paginas;
    pagina->numero_pagina = numero_pagina;

    return pagina;
}

t_marco *marco_create(int pid, int numero_marco)
{
    t_marco *marco = malloc(sizeof(t_marco));

    marco->numero_marco = numero_marco;
    marco->pid = pid;

    return marco;
}

t_proceso *proceso_create(t_list *instrucciones, t_list *segmentos)
{
    t_proceso *proceso = malloc(sizeof(t_proceso));
    proceso->instrucciones = instrucciones;
    proceso->segmentos = segmentos;
    return proceso;
}

/* Destruccion */

void dispositivo_io_destroy(void *arg)
{
    t_dispositivo *dispositivo_io = (t_dispositivo *)arg;
    free(dispositivo_io->nombre);
    queue_destroy_and_destroy_elements(dispositivo_io->cola, pcb_destroy);
    free(dispositivo_io);
}

void instruccion_destroy(void *arg)
{
    instruccion *_instruccion = (instruccion *)arg;
    free(_instruccion->parametro1);
    free(_instruccion->parametro2);
    free(_instruccion);
}

void pcb_destroy(void *arg)
{
    t_pcb *pcb = (t_pcb *)arg;
    list_destroy_and_destroy_elements(pcb->instrucciones, instruccion_destroy);
    if (pcb->tabla_de_segmentos != NULL)
    {
        list_destroy_and_destroy_elements(pcb->tabla_de_segmentos, free);
    }
    if (pcb->tamanio_segmentos != NULL)
    {
        list_destroy_and_destroy_elements(pcb->tamanio_segmentos, free);
    }
    free(pcb->pagina_fault);
    free(pcb);
}

void pcb_memoria_destroy(t_pcb_memoria *pcb)
{
    if (pcb->segmentos != NULL)
    {
        list_destroy_and_destroy_elements(pcb->segmentos, free);
    }
    free(pcb);
}

void proceso_destroy(t_proceso *proceso)
{
    list_destroy_and_destroy_elements(proceso->instrucciones, instruccion_destroy);
    list_destroy_and_destroy_elements(proceso->segmentos, free);
    free(proceso);
}

/* Utils */

char *estado_to_string(estado_proceso estado)
{
    switch (estado)
    {
    case NEW:
        return "NEW";
        break;
    case READY:
        return "READY";
        break;
    case BLOCK:
        return "BLOCK";
        break;
    case EXEC:
        return "EXEC";
        break;
    case FINISH_EXIT:
        return "EXIT";
        break;
    case FINISH_ERROR:
        return "ERROR";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

cod_operacion string_to_cod_op(char *estado_string)
{
    if (string_equals_ignore_case(estado_string, "SET"))
        return SET;
    else if (string_equals_ignore_case(estado_string, "ADD"))
        return ADD;
    else if (string_equals_ignore_case(estado_string, "MOV_IN"))
        return MOV_IN;
    else if (string_equals_ignore_case(estado_string, "MOV_OUT"))
        return MOV_OUT;
    else if (string_equals_ignore_case(estado_string, "I/O"))
        return IO;
    else if (string_equals_ignore_case(estado_string, "EXIT"))
        return EXIT;
    else
        return UNKNOWN_OP;
}

char *pcb_to_string(t_pcb *pcb)
{
    char *pcb_string = string_new();
    char *pcb_estado = estado_to_string(pcb->estado);
    int cantidad_instrucciones = list_size(pcb->instrucciones);
    string_append_with_format(&pcb_string,
                              "PID: %d\nPPID: %d\nPC: %d\nESTADO: %s\nINTERRUPCION: %d\n\nAX= %d BX= %d \nCX= %d DX= %d \n\n",
                              pcb->pid,
                              pcb->socket_consola,
                              pcb->program_counter,
                              pcb_estado,
                              pcb->interrupcion,
                              pcb->registros.ax,
                              pcb->registros.bx,
                              pcb->registros.cx,
                              pcb->registros.dx);
    for (int i = 0; i < cantidad_instrucciones; i++)
    {
        char *instruccion = instruccion_to_string(list_get(pcb->instrucciones, i));
        string_append_with_format(&pcb_string, "%s\n", instruccion);
        free(instruccion);
    }

    return pcb_string;
}

char *instruccion_to_string(instruccion *instruccion)
{
    char *instruccion_string = string_new();
    char *parametro1 = instruccion->parametro1;
    char *parametro2 = instruccion->parametro2;
    switch (instruccion->operacion)
    {
    case SET:
        string_append_with_format(&instruccion_string, "SET  %s  %s", parametro1, parametro2);
        break;
    case ADD:
        string_append_with_format(&instruccion_string, "ADD  %s  %s", parametro1, parametro2);
        break;
    case MOV_IN:
        string_append_with_format(&instruccion_string, "MOV_IN  %s  %s", parametro1, parametro2);
        break;
    case MOV_OUT:
        string_append_with_format(&instruccion_string, "MOV_OUT  %s  %s", parametro1, parametro2);
        break;
    case IO:
        string_append_with_format(&instruccion_string, "I/O  %s  %s", parametro1, parametro2);
        break;
    case EXIT:
        string_append(&instruccion_string, "EXIT");
        break;
    default:
        string_append(&instruccion_string, "UNKNOWN");
        break;
    }

    return instruccion_string;
}

char *operacion_to_string(cod_operacion operacion)
{
    switch (operacion)
    {
    case ADD:
        return "ADD";
    case SET:
        return "SET";
    case MOV_IN:
        return "MOV_IN";
    case MOV_OUT:
        return "MOV_OUT";
    case IO:
        return "I/O";
    case EXIT:
        return "EXIT";
    default:
        return "Operador invalido";
    }
}

char *algoritmo_to_string(t_algoritmo operacion)
{
    switch (operacion)
    {
    case FIFO:
        return "FIFO";
    case FEEDBACK:
        return "FEEDBACK";
    case RR:
        return "RR";
    case LRU:
        return "LRU";
    case CLOCK:
        return "CLOCK";
    case CLOCK_M:
        return "CLOCK-M";
    default:
        return "Operador invalido";
    }
}

void set_valor_registro(t_pcb *pcb, char *parametro1, char *parametro2)
{
    uint32_t valor = atoi(parametro2);
    if (string_equals_ignore_case(parametro1, "ax"))
    {
        pcb->registros.ax = valor;
    }
    else if (string_equals_ignore_case(parametro1, "bx"))
    {
        pcb->registros.bx = valor;
    }
    else if (string_equals_ignore_case(parametro1, "cx"))
    {
        pcb->registros.cx = valor;
    }
    else if (string_equals_ignore_case(parametro1, "dx"))
    {
        pcb->registros.dx = valor;
    }
}

uint32_t obtener_valor_del_registro(t_pcb *pcb, char *parametro1)
{
    uint32_t valor_de_registro;
    if (string_equals_ignore_case(parametro1, "ax"))
    {
        valor_de_registro = pcb->registros.ax;
    }
    else if (string_equals_ignore_case(parametro1, "bx"))
    {
        valor_de_registro = pcb->registros.bx;
    }
    else if (string_equals_ignore_case(parametro1, "cx"))
    {
        valor_de_registro = pcb->registros.cx;
    }
    else if (string_equals_ignore_case(parametro1, "dx"))
    {
        valor_de_registro = pcb->registros.dx;
    }

    return valor_de_registro;
}

t_list *pcb_queue_to_pid_list(t_queue *queue)
{
    t_list *lista = list_create();
    for (int i = 0; i < queue_size(queue); i++)
    {
        t_pcb *pcb = (t_pcb *)queue_pop(queue);
        int *valor = &pcb->pid;
        list_add(lista, valor);
        queue_push(queue, pcb);
    }
    return lista;
}

char *list_to_string(t_list *list)
{
    char *string = string_new();
    for (int i = 0; i < list_size(list); i++)
    {
        int *num = (int *)list_get(list, i);
        if (i < list_size(list) - 1)
        {
            string_append_with_format(&string, "%d,", *num);
        }
        else
        {
            string_append_with_format(&string, "%d", *num);
        }
    }
    return string;
}

/* TIMMER */

void ejecutar_espera(int tiempo)
{
    usleep(tiempo * 1000);
}
