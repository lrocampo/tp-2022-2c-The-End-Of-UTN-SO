#include <mmu.h>


int obtener_desplazamiento_segmento(int dir_logica, int tam_max_segmento){
	return dir_logica % tam_max_segmento;
}

int obtener_desplazamiento_pagina(int dir_logica, int tam_max_segmento, int tamanio_pagina){
	int desplazamiento_segmento = obtener_desplazamiento_segmento(dir_logica, tam_max_segmento);
	int desplazamiento_pagina = desplazamiento_segmento % tamanio_pagina;
	return desplazamiento_pagina;
}

int obtener_nro_pagina(int dir_logica, int tam_max_segmento, int tamanio_pagina){
	int desplazamiento_segmento = obtener_desplazamiento_segmento(dir_logica, tam_max_segmento);
	int num_pagina = floor(desplazamiento_segmento / tamanio_pagina);
    return num_pagina;
}

int obtener_nro_segmento(int dir_logica, int tam_max_segmento){
	return floor(dir_logica / tam_max_segmento);
}



