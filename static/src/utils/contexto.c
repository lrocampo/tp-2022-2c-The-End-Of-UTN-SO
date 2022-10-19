/*
 * contexto.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#include <utils/contexto.h>

t_pcb* pcb_create(t_list* instrucciones, uint32_t pid, int socket){
    t_pcb* pcb = malloc(sizeof(t_pcb));

    pcb->estado = NEW;
    pcb->instrucciones = instrucciones;
    pcb->pid = pid;
    pcb->socket_consola = socket;
    pcb->program_counter = 0;
    pcb->interrupcion = false;
    pcb->registros.ax = 0;
    pcb->registros.bx = 0;
    pcb->registros.cx = 0;
    pcb->registros.dx = 0;
    pcb->tabla.indice_tabla_paginas = 0;
    pcb->tabla.nro_segmento = 0;
    pcb->tabla.tamanio_segmento = 0;

    return pcb;
}

void pcb_destroy(t_pcb* pcb){
    list_destroy_and_destroy_elements(pcb->instrucciones, free);
    free(pcb);
}

char* estado_to_string(estado_proceso estado){
    switch (estado)
    {
    case NEW:
        return "NEW";
        break;
    case READY:
        return "READY";
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

char* pcb_to_string(t_pcb* pcb){
    char* pcb_string = string_new();
    char* pcb_estado = estado_to_string(pcb->estado);

    int cantidad_instrucciones = list_size(pcb->instrucciones);
    string_append_with_format(&pcb_string,
        "PID: %d\nPPID: %d\nPC: %d\nESTADO: %s\nINTERRUPCION: %d\n\nAX= %d BX= %d \nCX= %d DX= %d \n\nIDX: %d  N.PAG: %d  TAM: %d\n\n",
        pcb->pid,
        pcb->socket_consola,
        pcb->program_counter,
        pcb_estado,
        pcb->interrupcion,
        pcb->registros.ax,
        pcb->registros.bx,
        pcb->registros.cx,
        pcb->registros.dx,
        pcb->tabla.indice_tabla_paginas,
        pcb->tabla.nro_segmento,
        pcb->tabla.tamanio_segmento
        );
        for(int i = 0; i < cantidad_instrucciones; i++){
            char* instruccion = instruccion_to_string(list_get(pcb->instrucciones, i));
            string_append_with_format(&pcb_string,"%s\n",instruccion);
            free(instruccion);
        }

    return pcb_string;
}

char* instruccion_to_string(instruccion* instruccion){
    char* instruccion_string = string_new();
    char* parametro1 = instruccion->parametro1;
    char* parametro2 = instruccion->parametro2;
    switch (instruccion->operacion)
    {
    case SET:
        string_append_with_format(&instruccion_string,"SET  %s  %s", parametro1, parametro2);
        break;
    case ADD:
        string_append_with_format(&instruccion_string,"ADD  %s  %s", parametro1, parametro2);
        break;
    case MOV_IN:
        string_append_with_format(&instruccion_string,"MOV_IN  %s  %s", parametro1, parametro2);
        break;
    case MOV_OUT:
        string_append_with_format(&instruccion_string,"MOV_OUT  %s  %s", parametro1, parametro2);
        break;
    case IO:
        string_append_with_format(&instruccion_string,"I/O  %s  %s", parametro1, parametro2);
        break;
    case EXIT:
        string_append(&instruccion_string,"EXIT");
        break;
    default:
        string_append(&instruccion_string,"UNKNOWN");
        break;
    }

    return instruccion_string;
}