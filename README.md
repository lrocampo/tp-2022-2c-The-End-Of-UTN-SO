# THE END OF UTN SO 

![The End Of UTN SO Image](the_end_of_utn_so.JPG)

_YOU SHALL **(NOT)** COMPILE_ 

## Integrantes
* Cecilia Diaz
* Mauro De Marco
* Nahuel Gimenez
* Lautaro Ocampo
* Valentina Rau

## Cómo buildear el TP
Seguir los siguientes pasos:
```sh
git clone https://github.com/valentinarau/so-deploy-the-end-of-utn-so.git

cd so-deploy

./deploy.sh -p=kernel -p=memoria -p=cpu -p=consola tp-2022-2c-The-End-Of-UTN-SO
```
Configurar los IPs:
```sh
cd tp-2022-2c-The-End-Of-UTN-SO

./ip_binding.sh <IP_KERNEL> <IP_MEMORIA> <IP_CPU>
```
