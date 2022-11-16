#include<tlb.h>

t_list* tabla_tlb;

int buscar_en_tlb(int pid, int numero_pagina) {
    t_entrada_tlb entrada = list_get(tabla_tlb, numero_pagina);
    bool contiene_pagina = list_any_satisfy(tabla_tlb, (void*)_numero_pagina_solicitado(numero_pagina, pid));
    if(contiene_pagina){
        actualizar_instante_de_ultima_referencia(entrada);
        return entrada->marco;
    }
    else{
        return -1
    }
}
/*if(tabla_tlb >= tlb_config->entradas_tlb){
            switch(tlb_config->reemplazo_tlb) {
		case LRU:
			fifo(entrada);
			break;
		case FIFO:
			lru(entrada);
			break;
		default:
			error_show("Error, algoritmo desconocido.");												
	        }
        }*/


void* lru(t_entrada_tlb entrada_nueva){
    t_entrada_tlb* entrada_menos_usada = (t_entrada_tlb*) list_get_minimum(tabla_tlb, (void*)_menor_instante_entre_dos);
    list_remove(tabla_tlb, );
    list_add(table_tlb, entrada_nueva);
}

static void* _menor_instante_entre_dos(t_entrada* e1, t_entrada* e2){
    return e1->instante_de_ultima_referencia <= e2->instante_de_ultima_referencia ? e1 : e2;  
}





bool actualizar_tlb(int pid, int numero_pagina, int segmento) {
    return true;
}
bool _numero_pagina_solicitado(int numero_pagina, int pid, void* entrada){
    return ((t_entrada_tlb*)entrada)->pagina == numero_pagina && ((t_entrada_tlb*)entrada)->pid == pid;
}

void* fifo(t_entrada_tlb entrada_nueva){
    list_remove(tabla_tlb, 0);
    list_add(table_tlb, entrada_nueva);
}
