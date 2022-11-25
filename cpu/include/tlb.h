#include <cpu_utils.h>

int buscar_en_tlb(int, int, int);
void actualizar_tlb(int, int, int, int);
t_entrada_tlb *obtener_victima_a_reemplazar();
void ejecutar_reemplazo();
void log_tlb();
void rajar_entrada(t_entrada_tlb *);
void *minimo_instante_de_referencia(t_entrada_tlb *, t_entrada_tlb *);
void limpiar_proceso_de_la_tlb(int);