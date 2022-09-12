#include <consola.h>

int main(void){

	char* string = string_new();
	int conexion;
	char* ip = IP;
	char* puerto = PUERTO;
	char* valor = "holis, soy la consola";
	string_append(&string,"consola2\n");
	puts(string);

	//puts(IP);

	conexion = crear_conexion(ip, puerto);

	enviar_mensaje(valor, conexion);

	liberar_conexion(conexion);
	puts("termino consola\n");


	return 0;
}
