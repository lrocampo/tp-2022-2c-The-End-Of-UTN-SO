#include <planificacion.h>

/* Planificacion */

void planificacion_init()
{
	algoritmo = kernel_config->algoritmo;
	dispositivos_io = kernel_config->dispositivos_io;
	cantidad_dispositivos = list_size(dispositivos_io);
	pid_actual = 0;
	pthread_mutex_init(&pid_mutex, NULL);
	/* Al encender el kernel, arrancamos con pid 0 */
	sem_init(&multiprogramacion, 0, kernel_config->grado_multiprogramacion);

	largo_plazo_init();
	corto_plazo_init();
	dispositivos_io_init();
}

/* Largo Plazo */

void largo_plazo_init()
{
	cola_new_pcbs = queue_create();
	cola_exit_pcbs = queue_create();

	sem_init(&procesos_new, 0, 0);
	sem_init(&procesos_finalizados, 0, 0);
	pthread_mutex_init(&cola_new_pcbs_mutex, NULL);
	pthread_mutex_init(&cola_exit_pcbs_mutex, NULL);

	pthread_create(&th_rajar_pcb, NULL, &rajar_pcb, NULL);
	pthread_create(&th_conexiones, NULL, &atender_nueva_consola, NULL);
	pthread_detach(th_rajar_pcb);
}

void *atender_nueva_consola(void *arg)
{
	t_pcb *pcb;
	t_list *instrucciones;
	t_list *segmentos;
	t_proceso *proceso;
	while (1)
	{
		log_debug(kernel_logger, "Soy Kernel. Esperando conexion...");
		int consola_fd = esperar_cliente(kernel_server_fd);
		log_debug(kernel_logger, "se conecto un cliente");
		cod_mensaje cod_msj = recibir_operacion(consola_fd);
		if (cod_msj == PROCESO)
		{
			proceso = deserializar_proceso(consola_fd);
			instrucciones = proceso->instrucciones;
			segmentos = proceso->segmentos;
			log_debug(kernel_logger, "Recibí %d instrucciones", list_size(instrucciones));
			log_debug(kernel_logger, "Recibí %d segmentos", list_size(segmentos));
			pcb = pcb_create(proceso, siguiente_pid(), consola_fd);
			safe_pcb_push(cola_new_pcbs, pcb, &cola_new_pcbs_mutex);
			log_info(kernel_logger, "Se crea el proceso %d en NEW", pcb->pid);
			free(proceso);
			sem_post(&procesos_new);
		}
		else
		{
			error_show("Mensaje desconocido");
		}
	}
}

void solicitar_creacion_estructuras_administrativas(t_pcb *pcb)
{
	t_pcb_memoria *pcb_memoria = malloc(sizeof(t_pcb_memoria));
	pcb_memoria->pid = pcb->pid;
	pcb_memoria->segmentos = pcb->tamanio_segmentos;
	pthread_mutex_lock(&conexion_memoria_mutex);
	enviar_pcb_memoria(pcb_memoria, conexion_memoria);
	cod_mensaje mensaje = recibir_operacion(conexion_memoria);

	if (mensaje == OKI_ESTRUCTURAS)
	{
		t_list *indices = recibir_indices_tabla_paginas(conexion_memoria);
		pthread_mutex_unlock(&conexion_memoria_mutex);
		pcb->tabla_de_segmentos = crear_tabla_segmentos(indices, pcb->tamanio_segmentos); // sacar cuando no se use
		log_debug(kernel_logger, "Se han creado las estructuras para el proceso %d, %d tablas", pcb->pid, list_size(pcb->tabla_de_segmentos));
		list_destroy_and_destroy_elements(indices, free);
		free(pcb_memoria);
	}
	else
	{
		pthread_mutex_unlock(&conexion_memoria_mutex);
		error_show("Error al crear estructuras");
		pthread_exit(NULL);
	}
}

t_list *crear_tabla_segmentos(t_list *indices, t_list *tamanio_segmentos)
{
	t_list *tabla = list_create();
	for (int i = 0; i < list_size(indices); i++)
	{
		char *tamanio = list_get(tamanio_segmentos, i);
		char *indice = list_get(indices, i);
		t_segmento *segmento = segmento_create(i, atoi(indice), atoi(tamanio));
		list_add(tabla, segmento);
	}
	return tabla;
}

void *transicion_new_a_ready(void *arg)
{
	while (1)
	{
		sem_wait(&procesos_new);
		sem_wait(&multiprogramacion);
		t_pcb *pcb = safe_pcb_pop(cola_new_pcbs, &cola_new_pcbs_mutex);
		solicitar_creacion_estructuras_administrativas(pcb);
		pasar_a_ready(pcb);
	}
}

void solicitar_finalizacion(t_pcb *pcb)
{
	cod_mensaje cod_msj_consola = FINALIZAR;
	cod_mensaje cod_msj_memoria = LIBERAR_ESTRUCTURAS;
	cambiar_estado(pcb, FINISH_EXIT);
	safe_pcb_push(cola_exit_pcbs, pcb, &cola_exit_pcbs_mutex);
	enviar_valor_con_codigo(pcb->pid, cod_msj_memoria, conexion_memoria);
	cod_msj_memoria = recibir_operacion(conexion_memoria);
	if (cod_msj_memoria == OKI_LIBERAR_ESTRUCTURAS)
	{
		enviar_datos(pcb->socket_consola, &cod_msj_consola, sizeof(cod_msj_consola));
	}
	sem_post(&procesos_finalizados);
}

void *rajar_pcb(void *arg)
{
	while (1)
	{
		sem_wait(&procesos_finalizados);
		t_pcb *pcb = safe_pcb_pop(cola_exit_pcbs, &cola_exit_pcbs_mutex);
		log_debug(kernel_logger, "PCB con id: %d ha finalizado.", pcb->pid);
		pcb_destroy(pcb);
		sem_post(&multiprogramacion);
	}
}

/* Corto Plazo */

void corto_plazo_init()
{
	cola_ready_FIFO_pcbs = queue_create();
	cola_ready_RR_pcbs = queue_create();

	sem_init(&procesos_ready, 0, 0);
	sem_init(&proceso_page_fault, 0, 0);
	pthread_mutex_init(&cola_ready_RR_pcbs_mutex, NULL);
	pthread_mutex_init(&cola_ready_FIFO_pcbs_mutex, NULL);

	pthread_create(&th_ejecucion, NULL, &planificar_ejecucion, NULL);
	pthread_create(&th_transiciones_ready, NULL, &transicion_new_a_ready, NULL);

	pthread_detach(th_transiciones_ready);
	pthread_detach(th_ejecucion);
}

void pasar_a_ready(t_pcb *pcb)
{
	push_ready_pcb(pcb);
	cambiar_estado(pcb, READY);
	log_cola_ready();
	sem_post(&procesos_ready);
}

void *planificar_ejecucion(void *arg)
{
	while (1)
	{
		t_pcb *pcb = seleccionar_pcb();
		planificar_interrupcion(pcb);
		enviar_pcb(pcb, conexion_cpu_dispatch);
		pcb_destroy(pcb);
		pcb = obtener_proceso_ejecutado();
		analizar_contexto_recibido(pcb);
		dirigir_proceso_ejecutado(pcb);
	}
}

t_pcb *seleccionar_pcb()
{
	sem_wait(&procesos_ready);
	t_pcb *pcb = pop_ready_pcb();
	log_debug(kernel_logger, "Estado PCB: %d", pcb->estado);
	cambiar_estado(pcb, EXEC);
	log_debug(kernel_logger, "Enviando PCB");
	return pcb;
}

void planificar_interrupcion(t_pcb *pcb)
{
	if (pcb->con_desalojo)
	{
		iniciar_interrupcion();
		sem_post(&interrupcion_quantum);
	}
}

void iniciar_interrupcion()
{
	pthread_create(&th_timer, NULL, &enviar_interrupt, NULL);
	pthread_detach(th_timer);
}

void *enviar_interrupt(void *arg)
{
	while (1)
	{
		sem_wait(&interrupcion_quantum);
		cod_mensaje mensaje = INTERRUPCION;
		ejecutar_espera(kernel_config->quantum_RR);
		enviar_datos(conexion_cpu_interrupt, &mensaje, sizeof(mensaje));
	}
}

t_pcb *obtener_proceso_ejecutado()
{
	t_pcb *pcb;
	int cod_op = recibir_operacion(conexion_cpu_dispatch);
	if (cod_op == PCB)
	{
		pcb = recibir_pcb(conexion_cpu_dispatch);
		return pcb;
	}
	else
	{
		error_show("Error al recibir PCB");
		pthread_exit(NULL);
	}
}

void analizar_contexto_recibido(t_pcb *pcb)
{
	if (pcb->con_desalojo)
	{
		pthread_cancel(th_timer);
		log_debug(kernel_logger, "Interrupcion cancelada por vuelta del PCB");
	}
	if (pcb->interrupcion)
	{ // cuando hay fin de quantum
		log_info(kernel_logger, "PID: %d - Desalojado por fin de Quantum", pcb->pid);
		pcb->interrupcion = false;
		pcb->con_desalojo = false;
	}
	if (pcb->page_fault)
	{
		cambiar_estado(pcb, BLOCK);
		pthread_create(&th_manejo_page_fault, NULL, &manejar_page_fault, (void *)pcb);
		pthread_detach(th_manejo_page_fault);
		sem_post(&proceso_page_fault);
	}
}

void *manejar_page_fault(void *arg)
{
	while (1)
	{
		sem_wait(&proceso_page_fault);
		t_pcb *pcb = (t_pcb *)arg;
		bool segmento_by_pagina(t_segmento * segmento)
		{
			return segmento->indice_tabla_paginas == pcb->pagina_fault->indice_tabla_de_pagina;
		}
		t_segmento *segmento_page_fault = list_find(pcb->tabla_de_segmentos, (void *)segmento_by_pagina);
		log_info(kernel_logger, "Page Fault PID: %d - Segmento: %d - Pagina: %d", pcb->pid, segmento_page_fault->nro_segmento, pcb->pagina_fault->numero_pagina);
		log_debug(kernel_logger, "Enviando pagina fault: %d", pcb->pagina_fault->numero_pagina);
		pthread_mutex_lock(&conexion_memoria_mutex);
		enviar_pagina(pcb->pagina_fault, conexion_memoria);
		cod_mensaje cod_mensaje_memoria = recibir_operacion(conexion_memoria); // consultar si es correcto
		if (cod_mensaje_memoria == OKI_PAGINA)
		{
			pthread_mutex_unlock(&conexion_memoria_mutex);
			log_debug(kernel_logger, "page fault solucionado pid: %d", pcb->pid);
			pasar_a_ready(pcb);
			pthread_exit(NULL);
		}
		else
		{
			error_show("Error en page fault");
			pthread_mutex_unlock(&conexion_memoria_mutex);
			pthread_exit(NULL);
		}
	}
}

void dirigir_proceso_ejecutado(t_pcb *pcb)
{ // corto plazo // tener en cuenta page default ya que no deberiamos modificar el program counter
	if (pcb->segmentation_fault)
	{
		solicitar_finalizacion(pcb);
	}
	else if (!pcb->page_fault)
	{
		instruccion *ultima_instruccion = obtener_ultima_instruccion(pcb);
		switch (ultima_instruccion->operacion)
		{
		case EXIT:
			solicitar_finalizacion(pcb);
			break;
		case IO:
			solicitar_io(pcb, ultima_instruccion);
			break;
		default:
			pasar_a_ready(pcb);
			break;
		}
	}
}