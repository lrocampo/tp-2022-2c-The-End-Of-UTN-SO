#include <utils/contexto.h>

t_pcb* pcb_create(){
    t_pcb* pcb = malloc(sizeof(t_pcb));

    pcb->estado = NEW;
    pcb->instrucciones = list_create();
    pcb->pid = rand();
    pcb->program_counter = 0;
    pcb->registros.ax = 0;
    pcb->registros.bx = 0;
    pcb->registros.cx = 0;
    pcb->registros.dx = 0;
    pcb->tabla.indice_tabla_paginas = 0;
    pcb->tabla.nro_segmento = 0;
    pcb->tabla.tamanio_segmento = 0;

    return pcb;
}