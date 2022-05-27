//JON GARCIA GONZALEZ 
#include <stdio.h>
#include <stdlib.h>
#include "pist2.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define ERROR 100
#define minPistoleros 2
#define maxPistoleros 26

void semOp(int nSem, int n);
void sendMsg(long tipo, char msg);
void finalizar();

//VARIABLES IPCS
int semID=-1; //Semaforos

int memID=-1; //Memoria Compartida
char *memPunt; //puntero a memoria compartida
int buzon=-1; //BUZON

struct tipo_mensaje{
		long tipo;		
		char msg;
	}m;
	
//CTRL C
struct sigaction hh, hv;

//ENCINA	
union semun {
	int val;
	struct semid_ds *buf;
};

int pidPadre;

void handler();
void handler(){
	if(pidPadre==getpid()){
	if(-1!=buzon){
		msgctl(buzon, IPC_RMID, 0);
	}
	if(-1!=semID){
		semctl(semID,0,IPC_RMID);
	}	
	if(-1!=memID){
		shmctl(memID, IPC_RMID, NULL);
		shmdt(memPunt);
	}
	}
	exit(0);
}
int main(int argc, char *argv[]){
	
	pidPadre=getpid();
	int nPistoleros;
	int velocidad;
	int semilla=0;
	char letras[26]={'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
	pid_t PID;
	
	char victima; char name; //Letra de la victima y letra del propio pistolero
	int newPist;
	
	int j,i,id;
	
	//CTRL C
	hh.sa_handler=handler;
	sigemptyset(&hh.sa_mask);
	hh.sa_flags=0;

	if(-1==sigaction(SIGINT, &hh  ,&hv)){
		pon_error("Error handler");
		exit(ERROR);
	}
	
	//Paso de argumentos
	if (argc > 4 || argc < 3){
		fprintf(stderr, "\n Parametros incorrecto\n");
		exit(ERROR);
		}else{
		if(argc==4){
			nPistoleros=atoi(argv[1]);
			velocidad=atoi(argv[2]);
			semilla=atoi(argv[3]);
		}else{
			nPistoleros=atoi(argv[1]);
			velocidad=atoi(argv[2]);
			semilla=0;	
		}
		if(nPistoleros<minPistoleros || nPistoleros>maxPistoleros){
			fprintf(stderr, "\n Numero de pistoleros incorrecto (2-26)\n");
			exit(ERROR);
			}
		if(velocidad<0){
			fprintf(stderr, "\n La velocidad tiene que ser 0 o mayor\n");
			exit(ERROR);
		}
	}
	
	//Creacion IPCS		
	//SEmAFOROS	
	
	union semun sem1, sem2, sem3, sem4, sem5, sem6, sem7;
	sem1.val=nPistoleros;
	sem2.val=nPistoleros;
	sem3.val=nPistoleros;
	sem4.val=nPistoleros;
	sem5.val=nPistoleros;
	sem6.val=1;
	sem7.val=nPistoleros;
	
	semID = semget(IPC_PRIVATE, 8, IPC_CREAT | 0600);	
	if(semID==-1){
		pon_error("Error al crear el lote de semaforos");
		exit(ERROR);
	}
	//INICIALIZAR SEMAFOROS
	//PRIMER SEMAFORO
	if(semctl(semID,1,SETVAL, sem1)==-1) {
		pon_error("Error al inicializar semaforo 1");
		exit(ERROR);
	}
	//SEGUNDO SEMAFORO
	if(semctl(semID,2,SETVAL, sem2)==-1) {
		pon_error("Error al inicializar semaforo 2");
		exit(ERROR);
	}
	//TERCER SEMAFORO
	if(semctl(semID,3,SETVAL, sem3)==-1) {
		pon_error("Error al inicializar semaforo 3");
		exit(ERROR);
	}
	//CUARTO SEMAFORO
	if(semctl(semID,4,SETVAL, sem4)==-1) {
		pon_error("Error al inicializar semaforo 4");
		exit(ERROR);
	}
	//QUINTO SEMAFORO
	if(semctl(semID,5,SETVAL, sem5)==-1) {
		pon_error("Error al inicializar semaforo 5");
		exit(ERROR);
	}
	//SEXTO SEMAFORO
	if(semctl(semID,6,SETVAL, sem6)==-1) {
		pon_error("Error al inicializar semaforo 6");
		exit(ERROR);
	}
	//SEPTIMO SEMAFORO
	if(semctl(semID,7,SETVAL, sem7)==-1) {
		pon_error("Error al inicializar semaforo 7");
		exit(ERROR);
	}
	//Memoria compartida
	memID = shmget(IPC_PRIVATE, (256+(sizeof(int)*29)), IPC_CREAT | 0600);
	memPunt=(char *) shmat(memID, 0, 0);
	if(memID==-1 || memPunt == NULL){
		pon_error("Error al crear memoria compartida");
		exit(ERROR);
	}
	
	int *flPist = (int *)(memPunt+256);
	int *vivosPist = (int *)(flPist+26);
	int *coordinador = (int *)(flPist+27);
	int *pidSuperviviente = (int *)(flPist+28);
	int *listos = (int *)(flPist+29);
	//Inicializar valores de la MEM COMPARTIDA
	for(i=0;i<nPistoleros;i++){
		flPist[i]=1;
	}
	if(nPistoleros!=26){	
		for(i=nPistoleros;i<26;i++){ flPist[i]=0;}
	}
	*vivosPist = nPistoleros;
	*coordinador =0;
	*pidSuperviviente=0;
	*listos=0;
	//BUZON	
	buzon=msgget(IPC_PRIVATE, IPC_CREAT | 0600);
	if(buzon==-1){
		pon_error("Error al crear buzon");
		exit(ERROR);
	}
	
	

	if(PIST_inicio(nPistoleros, velocidad, semID, memPunt, semilla)==-1){
		pon_error("ERROR PIST INICIO");
		exit(ERROR);
		}
	
	//CREACION PISTOLEROS  ,semctl(semID,2,GETVAL, 0)
	
	for(i=0;i<nPistoleros;i++){
			
				
		PID = fork();
		if(PID==-1){ 
				pon_error("ERROR FORK");
				exit(ERROR);
		}
		if(PID==0){
				
				newPist = PIST_nuevoPistolero(letras[i]);
				name = letras[i];
				id=i;
				
				
				
				break;
		}
					
	}
	
	switch(PID){
			case -1: 
				pon_error("ERROR FORK");
				exit(ERROR);
			case 0:
			while(*vivosPist!=0){
			
				semOp(1,-1);
				semOp(1,0);//waitzero
				
				//Comprobar si es el coordinador
				if(*coordinador==id){
									
					//Comprobar que no es el unico vivo 
					if(*vivosPist==1){ 
						*pidSuperviviente= getpid();
						semOp(7,-1);						
						exit(0);
				}}
				
				//esperar a que se hayan creado todos los pistoleros para que apunten
				
				//una vez esten todos creados que apunten
				victima = PIST_vIctima();
				
				//El coordinador mandara un mensaje que los demas pistoleros estaran esperando para poder disparar
				semOp(2,-1);
				semOp(2,0);//WAIT ZERO
				if(*coordinador==id){
					//MANDAR SEÑAL DE QUE DISPAREN
					for(j=0;j<*vivosPist;j++){	
						sendMsg(1,name);
					}				
				}
				
				semOp(3,-1);
				semOp(3,0); //waitZero
				
							
				if(msgrcv(buzon,&m,sizeof(struct tipo_mensaje)-sizeof(long),1,0)==-1){
					pon_error("Error al recibir msg"); exit(ERROR);}
				
				
				PIST_disparar(victima);
				sendMsg(victima,name);
					
				semOp(4,-1);
				semOp(4,0);//WAIT ZERO	
								
				if(msgrcv(buzon,&m,sizeof(struct tipo_mensaje)-sizeof(long),name,IPC_NOWAIT)!=-1){
					//SI reciben mensaje se mueren
					PIST_morirme();
					
					flPist[id]=0;
					*vivosPist=*vivosPist - 1;
					semOp(5,-1);
					
					semOp(7,-1);//SEMAFORO PARA EL PADRE
					
						
					exit(0);
				}
				
				semOp(5,-1);
										
				semOp(5,0);//WAIT ZERO ESPERANDO QUE TODOS LOS PROCESOS HAYAN TERMINADO DE MATARSE
				*listos=*listos+1;
				
				
				
				 //Comprobar si murio el coordinador
				 semOp(6,-1);
				 if(flPist[*coordinador]==0){
				 	for(j=1;j<nPistoleros;j++){
				 		if(flPist[*coordinador+j]==1){
				 			*coordinador= *coordinador+j;
				 			break;
				 		}
				 	}
				 	
				 }
				 //Devolver a los semaforos con lo que hago wait zero el valor del numero de procesos que estan vivos
				 if(*listos==*vivosPist){
				 	semOp(1,*vivosPist);
					semOp(2,*vivosPist);
					semOp(3,*vivosPist);
					semOp(4,*vivosPist);
					semOp(5,*vivosPist);
					*listos=0;
					
				 }
				 
				 semOp(6,1);	
				
				
				
				}
			default:
				
				semOp(7,0);
				
				finalizar(*pidSuperviviente);
				
	}
}

void semOp(int nSem, int n){
	struct sembuf semOPE;
	
	
	semOPE.sem_num = nSem;
	semOPE.sem_op = n;
	semOPE.sem_flg = 0;
	
	if(semop(semID,&semOPE,1)==-1){
		char cadena[200];
		int fd=open("ErroresSemaforos.txt",O_WRONLY | O_CREAT | O_TRUNC);
		pon_error("Error opSEm");
		sprintf(cadena, "\n  me cago en: %d ", nSem);
		write(fd,cadena,strlen(cadena));
		exit(ERROR);
	}
}		

void finalizar(int valor){

	PIST_fin();
	if(-1!=buzon){
		msgctl(buzon, IPC_RMID, 0);
	}
	if(-1!=semID){
		semctl(semID,0,IPC_RMID);
	}	
	if(-1!=memID){
		shmctl(memID, IPC_RMID, NULL);
		shmdt(memPunt);
	}
	exit(valor);
}	
void sendMsg(long tipo, char msg){
	m.tipo = tipo; m.msg = msg;
	if(msgsnd(buzon,&m,sizeof(struct tipo_mensaje)-sizeof(long),0) == -1){pon_error("Error al mandar msg"); exit(ERROR);}
}
  	




