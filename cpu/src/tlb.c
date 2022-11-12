#include<tlb.h>

int buscar_en_tlb(int pid, int numero_pagina, int segmento) {
    return SEG_FAULT;
}

bool actualizar_tlb(int pid, int numero_pagina, int segmento) {
    return true;
}