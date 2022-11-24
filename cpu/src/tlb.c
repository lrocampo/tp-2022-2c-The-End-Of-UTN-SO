#include <tlb.h>

int buscar_en_tlb(int pid, int numero_pagina, int segmento)
{

    bool where_entrada_and_pid_is(t_entrada_tlb * entrada)
    {
        return entrada->pid == pid && entrada->pagina == numero_pagina && entrada->segmento == segmento;
    }

    t_entrada_tlb *entrada = list_find(tabla_tlb, (void *)where_entrada_and_pid_is);

    if (entrada == NULL)
    {
        log_debug(cpu_logger, "TLB MISS");
        return -1;
    }
    log_debug(cpu_logger, "TLB HIT");
    instante_de_referencia_actual++;
    entrada->instante_de_ultima_referencia = instante_de_referencia_actual;
    return entrada->marco;
}

void actualizar_tlb(int pid, int numero_pagina, int segmento, int marco)
{
    if (cpu_config->entradas_tlb > 0)
    {
        t_entrada_tlb *nueva_entrada = malloc(sizeof(t_entrada_tlb));
        int cantidad_entradas_actual = list_size(tabla_tlb);
        instante_de_referencia_actual++;
        nueva_entrada->instante_de_ultima_referencia = instante_de_referencia_actual;
        nueva_entrada->pid = pid;
        nueva_entrada->pagina = numero_pagina;
        nueva_entrada->segmento = segmento;
        nueva_entrada->marco = marco;
        if (cantidad_entradas_actual == cpu_config->entradas_tlb)
        {
            log_debug(cpu_logger, "Reemplazando pagina");
            ejecutar_reemplazo();
        }
        log_debug(cpu_logger, "Entrada agregada tlb");
        list_add(tabla_tlb, nueva_entrada);
        log_tlb();
    }
}

void log_tlb()
{
    for (int i = 0; i < list_size(tabla_tlb); i++)
    {
        t_entrada_tlb *entrada = list_get(tabla_tlb, i);
        log_info(cpu_logger, " %d |PID: %d |SEGMENTO: %d |PAGINA: %d |MARCO: %d", i, entrada->pid, entrada->segmento, entrada->pagina, entrada->marco);
    }
}

void ejecutar_reemplazo()
{
    t_entrada_tlb *entrada_a_reemplazar = obtener_victima_a_reemplazar();
    log_debug(cpu_logger, "entrada a reemplazar con pid %d, pag: %d, seg: %d", entrada_a_reemplazar->pid, entrada_a_reemplazar->pagina, entrada_a_reemplazar->segmento);
    rajar_entrada(entrada_a_reemplazar);
}

void rajar_entrada(t_entrada_tlb *entrada_a_rajar)
{

    bool where_instante_is(t_entrada_tlb * entrada)
    {
        return entrada->instante_de_ultima_referencia == entrada_a_rajar->instante_de_ultima_referencia;
    }

    list_remove_and_destroy_by_condition(tabla_tlb, (void *)where_instante_is, free);
}

t_entrada_tlb *obtener_victima_a_reemplazar()
{
    switch (cpu_config->reemplazo_tlb)
    {
    case FIFO:
        return list_get(tabla_tlb, 0);
    case LRU:
        return list_get_minimum(tabla_tlb, (void *)minimo_instante_de_referencia);
    default:
        error_show("Algoritmo tlb desconocido");
        exit(EXIT_FAILURE);
        break;
    }
}

void *minimo_instante_de_referencia(t_entrada_tlb *entrada1, t_entrada_tlb *entrada2)
{
    return entrada1->instante_de_ultima_referencia <= entrada2->instante_de_ultima_referencia
               ? entrada1
               : entrada2;
}

void limpiar_proceso_de_la_tlb(int pid)
{
    bool by_pid(t_entrada_tlb * entrada)
    {
        return entrada->pid == pid;
    }
    log_debug(cpu_logger, "Limpiando registros asociados al proceso %d", pid);

    list_remove_and_destroy_all_by_condition(tabla_tlb, (void *)by_pid, free);
    log_debug(cpu_logger, "Registros asociados al proceso %d liberados con exito.", pid);
    log_tlb();
}
