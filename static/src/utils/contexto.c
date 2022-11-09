/*
 * contexto.c
 *
 *  Created on: Sep 12, 2022
 *      Author: utnso
 */

#include <utils/contexto.h>

void instruccion_destroy(void* arg){
    instruccion* _instruccion = (instruccion*) arg;
    free(_instruccion->parametro1);
    free(_instruccion->parametro2);
    free(_instruccion);
}

t_pcb* pcb_create(t_proceso* proceso, uint32_t pid, int socket){
    t_pcb* pcb = malloc(sizeof(t_pcb));

    pcb->estado = NEW;
    pcb->instrucciones = proceso->instrucciones;
    pcb->pid = pid;
    pcb->socket_consola = socket;
    pcb->program_counter = 0;
    pcb->interrupcion = false;
    pcb->registros.ax = 0;
    pcb->registros.bx = 0;
    pcb->registros.cx = 0;
    pcb->registros.dx = 0;
    pcb->con_desalojo = false;
    pcb->tamanio_segmentos = proceso->segmentos;

    return pcb;
}

void pcb_destroy(void* arg){
    t_pcb* pcb = (t_pcb*) arg;
    list_destroy_and_destroy_elements(pcb->instrucciones, instruccion_destroy);
    if(pcb->tamanio_segmentos != NULL){
        list_destroy_and_destroy_elements(pcb->tamanio_segmentos, free);
    }
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

cod_operacion string_to_cod_op(char* estado_string){
    if(string_equals_ignore_case(estado_string, "SET"))
    return SET;
    else if(string_equals_ignore_case(estado_string, "ADD"))
    return ADD;
    else if(string_equals_ignore_case(estado_string, "MOV_IN"))
    return MOV_IN;
    else if(string_equals_ignore_case(estado_string, "MOV_OUT"))
    return MOV_OUT;
    else if(string_equals_ignore_case(estado_string, "I/O"))
    return IO;
    else if(string_equals_ignore_case(estado_string, "EXIT"))
    return EXIT;
    else return UNKNOWN_OP;
}

char* pcb_to_string(t_pcb* pcb){
    char* pcb_string = string_new();
    char* pcb_estado = estado_to_string(pcb->estado);
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
        pcb->registros.dx
        // pcb->tabla.indice_tabla_paginas,
        // pcb->tabla.nro_segmento,
        // pcb->tabla.tamanio_segmento
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

char* operacion_to_string(cod_operacion operacion) {
	switch(operacion) {
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

void set_valor_registro(t_pcb* pcb, char* parametro1, char* parametro2) {
    uint32_t valor = (uint32_t) atoi(parametro2);
	if(string_equals_ignore_case(parametro1, "ax")) {
		pcb->registros.ax = valor;
	}
	else if(string_equals_ignore_case(parametro1, "bx")) {
		pcb->registros.bx = valor;
	}
	else if(string_equals_ignore_case(parametro1, "cx")) {
		pcb->registros.cx = valor;
	}
	else if(string_equals_ignore_case(parametro1, "dx")) {
		pcb->registros.dx = valor;
	}	
}

uint32_t obtener_valor_del_registro(t_pcb* pcb, char* parametro1) {
	uint32_t valor_de_registro;
	if(string_equals_ignore_case(parametro1, "ax")) {
		valor_de_registro = pcb->registros.ax;
	}
	else if(string_equals_ignore_case(parametro1, "bx")) {
		valor_de_registro = pcb->registros.bx;
	}
	else if(string_equals_ignore_case(parametro1, "cx")) {
		valor_de_registro = pcb->registros.cx;
	}
	else if(string_equals_ignore_case(parametro1, "dx")) {
		valor_de_registro = pcb->registros.dx;
	}

	return valor_de_registro;
}