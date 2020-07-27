#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//AUTORES DEL CODIGO: Ivan Conde Carretero GII, Lara Sanchez Correa GII
//FECHA 10/12/19
//PRACTICA: Parking en C
sem_t sem;

int *plazas;
pthread_cond_t espera, esperaCamion;
pthread_mutex_t mutex;
int PLAZAS;
int PLANTAS;
int COCHES;
int CAMIONES;
int LIBRES;


void entrar (int idcoche, int idcaja){
	
	//sem_wait(&sem);//Ponemos el semaforo en rojo
	//Mostramos en pantalla el mensaje para avisar que un coche acaba de aparcar
	printf("ENTRADA: Coche %d aparca en %d. Plazas libre: %d\n" , idcoche, idcaja, LIBRES);
	//Al entrar, queda un plaza menos
	LIBRES--;
	int i;
	//Imprimimos el parking, con el coche que acaba de entrar aparcado
	printf("Parking: ");
	for(i=0; i<PLAZAS*PLANTAS; i++){
		if((i)%PLAZAS==0){
			printf("\n");
		}
		printf("[%d] " , plazas[i]);
	}
	printf("\n");
	//sem_post(&sem);//Ponemos el semaforo en verde
	fflush(stdout);	
}
	


void salir (int idcoche){
	//El coche se queda aparcado un periodo de tiempo de sleep(rand()%8)
	sleep(rand()%8);	
	sem_wait(&sem);//Ponemos el semaforo en rojo
	LIBRES++;//Actualizamos el numero de plazas, ya que el coche se ha ido del parking
	//Mostramos que el coche con el identidificador idcoche, se ha ido
	printf("SALIDA: Coche %d saliendo. Plazas libre: %d\n", idcoche, LIBRES);
	sem_post(&sem);//Ponemos el semaforo en verde
	fflush(stdout);
}


void entrarC (int idcoche, int idcaja){
	
	
	//sem_wait(&sem);//Ponemos el semaforo en rojo
	//Mostramos en pantalla el mensaje para avisar que un camion acaba de aparcar
	printf("ENTRADA: Camion %d aparca en %d. Plazas libre: %d\n" , idcoche, idcaja, LIBRES);
	//Al entrar un camion, queda dos plazas menos
	LIBRES = LIBRES-2;
	int i;
	//Imprimimos el parking, con el camion que acaba de entrar aparcado
	printf("Parking: ");
	for(i=0; i<PLAZAS*PLANTAS; i++){
		if((i)%PLAZAS==0){
			printf("\n");
		}
		printf("[%d] " , plazas[i]);
	}
	printf("\n");
	//sem_post(&sem);//Ponemos el semaforo en verde
	fflush(stdout);
	
}
	


void salirC (int idcoche){
	//El camion se queda aparcado un periodo de tiempo de sleep(rand()%8)
	sleep(rand()%8);	
	
	sem_wait(&sem);//Ponemos el semaforo en rojo
	LIBRES = LIBRES+2;//Actualizamos el numero de plazas, ya que el camion se ha ido del parking
	//Mostramos que el camion con el identidificador idcoche, se ha ido
	printf("SALIDA: Camion %d saliendo. Plazas libre: %d\n", idcoche, LIBRES);
	sem_post(&sem);//Ponemos el semaforo en verde
	fflush(stdout);
	
}
//Dicho metodo nos avisa si hay una plaza libre para un coche, si la hay nos indica la ubicacion
int plazalibre(void){
	int i;
	for(i=0; i<PLAZAS*PLANTAS; i++){
		if(plazas[i]==0){
			return i;
		}
	}//Si no hay plazas, devolvemos -1
	return -1;
}
//Dicho metodo nos avisa si hay dos plazas libres continuas, para un camion, si las hay nos indica la ubicacion
int plazalibreCamion(void){
	int i;
	for(i=0; i<PLAZAS*PLANTAS-1; i++){		
		if(plazas[i]==0 && plazas[i+1]==0 && (i%PLAZAS!=PLAZAS-1)){
		//Condiciones para que un camion entre correctamente y no este una plaza en una planta y otra en la de abajo
			return i;
		}
	}//Si no hay plazas, devolvemos -1
	return -1;
}
//Metodo que nos dira si tenemos dos plazas disponibles para un camion, lo usaremos para la inanicion
int disponibleCamion(){
	int i;
	for(i=0; i<PLAZAS*PLANTAS-1; i++){		
		if(plazas[i]==0 && plazas[i+1]==0 && (i%PLAZAS!=PLAZAS-1)){
			return 1;
		}
	}
	return 0;
}


void *hiloCamion(void *a){
	int id =*(int*)a;
	while(1){
		int posParking;
	
		//Cierro la puerta si esta abierta, sino me quedo esperando para poder entrar
		pthread_mutex_lock(&mutex);
					//DENTRO DE LA SECCION CRITICA
		//Si tengo plaza libre continuo, sino me quedo hasta que me manden una señal y me despierten
		while((posParking=plazalibreCamion())<0){
			pthread_cond_wait(&esperaCamion, &mutex);
		}
		//Tengo plaza para aparcar, asi que pongo mi identificador en dicha plaza
		plazas[posParking] = id;
		plazas[posParking+1] = id;
		//Llamo el metodo para imprimir la entrada y el parking
		entrarC(id, posParking);
		pthread_mutex_unlock(&mutex);//Abro la puerta, para que otros puedan entrar.
		
		//Metodo donde el camion se queda sleep, que sera el tiempo que se queda dentro del parking
		salirC(id);
		//Cierro la puerta si esta abierta, sino me quedo esperando para poder entrar
		pthread_mutex_lock(&mutex);
					//DENTRO DE LA SECCION CRITICA
		plazas[posParking]=0; //salgo de esa plaza
		plazas[posParking+1] = 0;//salgo de esa plaza
		
						//INANICION
		pthread_cond_broadcast(&esperaCamion);//Despierto a camiones para que entre alguno 
		sleep(5);	//Le doy un margen de dos, para que entren
		pthread_cond_signal(&espera); //Y sino, despierto un solo coche

		pthread_mutex_unlock(&mutex);//Abro la puerta, para que otros puedan entrar.

		pthread_exit(NULL);
	}
}

void *hiloC(void *a){
	int id =*(int*)a;
	while(1){
		
		int posParking;
		//Cierro la puerta si esta abierta, sino me quedo esperando para poder entrar
		pthread_mutex_lock(&mutex);
					//DENTRO DE LA SECCION CRITICA
		//Si tengo plaza libre continuo, sino me quedo hasta que me manden una señal y me despierten
		while((posParking=plazalibre())<0){
			pthread_cond_wait(&espera, &mutex);
		}
		//Tengo plaza para aparcar, asi que pongo mi identificador en dicha plaza
		plazas[posParking] = id;
		entrar(id, posParking);//Llamo el metodo para imprimir la entrada y el parking
		pthread_mutex_unlock(&mutex);//Abro la puerta, para que otros puedan entrar.
		//Metodo donde el coche se queda sleep, que sera el tiempo que se queda dentro del parking
		salir(id);
		//Cierro la puerta si esta abierta, sino me quedo esperando para poder entrar
		pthread_mutex_lock(&mutex);
					//DENTRO DE LA SECCION CRITICA
		plazas[posParking]=0;//salgo de esa plaza
						//INANICION
		if(disponibleCamion()==1){//Si hay dos plazas continuas entro
			pthread_cond_broadcast(&esperaCamion);	//Despierto a camiones para que entre alguno 
			sleep(5);	//Le doy un margen de dos, para que entren	
		}
		pthread_cond_signal(&espera);//Y sino, despierto un solo coche
		pthread_mutex_unlock(&mutex);//Abro la puerta, para que otros puedan entrar.
	
		pthread_exit(NULL);
	}
}
	

int main(int argc, char *argv[]){

	int i;
	int *ids;
	int *idsC;
	pthread_t *th;
	pthread_t *thC;
	

	if(argc==1){
		//NO SE LE PASA NINGUN ARGUMENTO, PONEMOS VALORES POR DEFECTO
		PLAZAS = 4;
		PLANTAS = 1;
		COCHES = PLAZAS*3;
		CAMIONES = PLAZAS*2;
		
	}else if(argc==5){
		PLAZAS= atoi(argv[1]);
		PLANTAS = atoi(argv[2]);
		COCHES = atoi(argv[3]);
		CAMIONES = atoi(argv[4]);	
	}else if(argc==4){	//Si no se le pasa el numero de camiones, este sera cero
		PLAZAS= atoi(argv[1]);
		PLANTAS = atoi(argv[2]);
		COCHES = atoi(argv[3]);
		CAMIONES = 0;	

	}else if(argc==3){ //Si no sel e pasa e numero de camiones sera cero, y si tampoco se le pasa 
			//el numero de coches, sera 2*PLANTAS*PLAZAS
		PLAZAS= atoi(argv[1]);
		PLANTAS = atoi(argv[2]);
		COCHES = 2*PLANTAS*PLAZAS;
		CAMIONES = 0;}
		LIBRES = PLANTAS*PLAZAS;

	plazas = malloc(sizeof(int)*(PLAZAS*PLANTAS)); //Este sera nuestro parking completo, con plazas y plantas
	ids = malloc(sizeof(int)*COCHES); //Identificadores de coches
	idsC = malloc(sizeof(int)*CAMIONES); //Identificadores de Camiones
	th = malloc(sizeof(pthread_t)*COCHES); 
	thC = malloc(sizeof(pthread_t)*CAMIONES);
	
	for(i=0; i<PLAZAS; i++){ //inicializamos todas las plazas a cero, ya que cero significan que están vacias.
		plazas[i] = 0;
	}

	sem_init(&sem, 0, 1); //Inicializamos nuestro semaforo a uno, dicho semaforo nos servira para imprimir en pantalla
			

	pthread_cond_init(&espera, NULL); //condicion para coches
	pthread_cond_init(&esperaCamion, NULL); //codiciones para camiones
	pthread_mutex_init(&mutex, NULL); //Mutex que nos servira para el interbloqueo, poder cerrar y abrir la puerta, 
	//para poder mirar un solo hilo en el array de plazas, asi evitar que dos hilos cojan la misma plaza


	if(CAMIONES>=COCHES){//Para el interbloqueo, hemos creado los hilos de coches y camiones de forma intercalada, para que no solo se metan
			//solo coches o solo todo camiones.
		for(i=0; i<CAMIONES; i++){
			if(COCHES>i){
				ids[i] = i+1;
				pthread_create(&th[i], NULL, hiloC, &ids[i]);						
			}
			idsC[i] = i+100;
			pthread_create(&thC[i], NULL, hiloCamion, &idsC[i]);
		}
	}else if(COCHES>CAMIONES){
		for(i=0; i<COCHES; i++){
			if(CAMIONES>i){
				idsC[i] = i+100;
				pthread_create(&thC[i], NULL, hiloCamion, &idsC[i]);									
			}
			ids[i] = i+1;
			pthread_create(&th[i], NULL, hiloC, &ids[i]);
		}

	}else if(CAMIONES==0){
		for(i=0; i<COCHES; i++){
			if(COCHES>i){
				idsC[i] = i+100;
				pthread_create(&thC[i], NULL, hiloCamion, &idsC[i]);						
			}
		}
	}
	

	if(CAMIONES>=COCHES){
		for(i=0; i<CAMIONES; i++){
			pthread_join(thC[i], NULL);	
			if(COCHES>i){
				pthread_join(th[i], NULL);
			}
		}
	}else{
		for(i=0; i<COCHES; i++){
			if(CAMIONES>i){
				pthread_join(thC[i], NULL);	
			}
			pthread_join(th[i], NULL);
		
		}
	}
	//Liberamos de memoria todo lo posible
	free(plazas);
	pthread_cond_destroy(&esperaCamion);
	pthread_cond_destroy(&espera);
	pthread_mutex_destroy(&mutex);
	sem_destroy(&sem);
	free(th);
	free(thC);
	exit(0);



}





