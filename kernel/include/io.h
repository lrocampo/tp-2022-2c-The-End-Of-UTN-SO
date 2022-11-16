#ifndef KERNEL_INCLUDE_IO_H_
#define KERNEL_INCLUDE_IO_H_

#include <kernel_utils.h>

/* IO */
void* ejecucion_io(void*);
void solicitar_dispositivo(t_pcb*,instruccion*);
void solicitar_io(t_pcb*, instruccion*);
void dispositivos_io_init();
t_dispositivo* obtener_dispositivo_por_nombre(char*);
void solicitud(instruccion*, t_pcb *);
void* solicitar_io_consola(void *);

#endif /* KERNEL_INCLUDE_IO_H_ */