#include <cpu_utils.h>

typedef struct {
    int pid;
    int segmento;
    int pagina;
    int marco;
    int instante_de_ultima_referencia;
} t_entrada_tlb;

extern t_list* tabla_tlb;
int buscar_en_tlb(int, int, int);

bool actualizar_tlb(int, int, int);