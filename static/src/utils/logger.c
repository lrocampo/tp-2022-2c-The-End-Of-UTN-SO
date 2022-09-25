#include <utils/logger.h>

t_log* iniciar_logger(char* ruta_log, char* modulo, int flag_consola, t_log_level log_level) {

	t_log* logger = log_create(ruta_log, modulo, flag_consola, log_level);

	if(logger == NULL){
		
		return 1;
	}
	else
		return logger;
}