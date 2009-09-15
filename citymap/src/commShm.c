/*
 * comm_shm.c
 *
 *  Created on: Sep 11, 2009
 *      Author:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//Nuestros includes
#include "comm.h"
#include "helper.h"
#include "sharedMem.h"
#include "semaphore.h"

//Id para Semaforos
#define SEMKEY_CLIENT (key_t)0x40
#define SEMKEY_SERVER (key_t)0x50

#define SEMKEY_LOG_CLIENT (key_t)0x80
#define SEMKEY_LOG_SERVER (key_t)0x90


//Definiciones para inicializar Shared memory
#define SHMKEY (key_t)0x100
#define SHMKEY_LOG (key_t)0x200
#define MAX_SHM MAP_SIZE

//Estructura a ser intercambiada en el shared memory
typedef struct mesg{
	int len;//indica el tamaño real de los datos en el buffer data
	int id; //info de identificacion, aqui solo hay un proceso, no es necesario
	char data[MAX_SHM];
}*mesgT;

#define FULL_PATH_LEN 100

struct comm{
	pid_t pid_server;
	pid_t pid_client;
	int tipoComm;
	char nombreServidor[FULL_PATH_LEN+1];

	//Definicion de variables para guardar los semaforos y shm usados
	int semClient;
	int semServer;
	mesgT mesgPtr;
};



//Funciones publicas para le manejo de comunicaciones

commT
iniciarComm( int tipoComm, char *nombreServidor )
{
	commT comm;

	if( ( comm = malloc( sizeof( struct comm) ) ) == NULL )
		return NULL;
	if( tipoComm == CLIENT || tipoComm == SERVER )
		comm->tipoComm = tipoComm;
	else
		comm->tipoComm = -1;

	comm->pid_client = -1;
	comm->pid_server = -1;
	comm->mesgPtr = NULL;

	strcpy(comm->nombreServidor, nombreServidor);
	/* TO create an UNIQUE KEY for each IPCs based on the well known server name
	 //"city" .. it can be any valid pathname in the systema(can be our deamon)
	 	key_t key = ftok(nombreServidor, "include/comm.h", 1);
	 	if(key == -1)
	 	{
	 		free(comm);
	 		FERROR("Unable to generate a valid key\n", NULL);
	 	}
	 	//...and then  sem_init( key, 1) ... 
	*/
	
	if( strcmp(nombreServidor, MAP_SERVER_NAME) == 0 )
	{
		//Incializo Semaforos
		//Como el cliente es quien inicia la comunicacion, lo pongo en 1
		comm->semClient = sem_init(SEMKEY_CLIENT, 1);
		//printf("Se inicializo semaforo cliente\n");//DEBUG
		comm->semServer = sem_init(SEMKEY_SERVER, 0);
		//printf("Se inicializo semaforo servidor\n");//DEBUG
	
		//Inicializo la shared memory
		if( (comm->mesgPtr = getMem(SHMKEY_LOG, sizeof(*(comm->mesgPtr))) ) == NULL )
			fatal("Error al inicializar memoria compartida\n");
	
	}
	else
	{	
		//Incializo Semaforos
		//Como el cliente es quien inicia la comunicacion, lo pongo en 1
		comm->semClient = sem_init(SEMKEY_LOG_CLIENT, 1);
		//printf("Se inicializo semaforo cliente\n");//DEBUG
		comm->semServer = sem_init(SEMKEY_LOG_SERVER, 0);
		//printf("Se inicializo semaforo servidor\n");//DEBUG
	
		//Inicializo la shared memory
		if( (comm->mesgPtr = getMem(SHMKEY_LOG, sizeof(*(comm->mesgPtr))) ) == NULL )
			fatal("Error al inicializar memoria compartida\n");
	}
	//Inicializo la shared mem con valores que indican que no hay datos nuevos
	comm->mesgPtr->len = 0;
	comm->mesgPtr->id = -1;
	//comm->mesgPtr->data[]
	
	//printf("Se inicializo SharedMem\n");//DEBUG
	return comm;
}

int
cerrarComm( commT comm )
{
	if( comm == NULL )
		return 1;

	//hace dettach
	liberar_conexion( comm );
	//borra los IPCs shared mem del sistema
	freeAllShm();

	return 0;
}

int
establecer_conexion( commT comm )
{
	//printf("establecer conexion(): entre a ESTABLECER CONEXION\n");//DEBUG
	//Tomo control de la shared mem
	//Si puedo entrar a la zona critica devuelvo un valor <>0 (pid_cliente)
	//Si no ... me bloqueo y espero para poder establecer la conexion
	//El servidor me avisara levantando su semaforo, lo que me desbloquea
	sem_down(comm->semClient);
	//printf("establecer_conexion(): process pid=%d Entre a zona critica\n", comm->pid_client);//DEBUG
	
	//SI no hay datos nuevos, entonces el servidor ya leyo datos y el shared
	//mem esta libre, entonces puedo establecer una nueva conexion.
	//Si el shared mem tiene tiene id, o el tamaño de datos es distinto de 
	//cero, entonces el server todavia no leyo datos de algun cliente
	//quiere decir que no puedo establecer la conexion, devuelvo CERO.
	if( comm->mesgPtr->id == -1 && comm->mesgPtr->len == 0 )		
		return comm->pid_client;
	else
	{
		//sem_up(comm->semClient);
		return 0;
	}
}

int
aceptar_conexion( commT comm )
{
	//printf("aceptar_conexion(): entre a ACEPTAR CONEXION\n");//DEBUG
	//Trato de tomar control sobre la shared mem
	//Si no lo logro ... hay un cliente escribiendo ... me duermo hasta
	//que el cliente termine y me avise que tengo una nueva conexion, entonces
	//la acepto mandando un valor distinto de cero (pid_server) y entro en
	//zona critica
	sem_down( comm->semServer );
	//printf("aceptar_conexion(): process pid=%d Entre a zona critica\n", comm->pid_server);//DEBUG
	
	//SI no hay datos nuevos, no hubo intento de conexion de ningun cliente
	if( comm->mesgPtr->id == -1 && comm->mesgPtr->len == 0 )
	{
		//sem_up( comm->semServer );
		return 0;
	}
	return comm->pid_server;
}

int
liberar_conexion( commT comm )
{
	if( comm == NULL )
		return 1;

	if( comm->mesgPtr == NULL )
		return 2;

	//Solo hace dettach de la shared mem
	freeMem( comm->mesgPtr );
	sem_rm( comm->semClient );
	sem_rm( comm->semServer	);
	return 0;
}

int
enviar_datos( commT comm, void *datos, int dim)
{
	//printf("enviar_datos(): entre a ENVIAR DATOS\n");//DEBUG
	//Asumo que antes ya consiguio acceso a la zona critica
	//Obligatorio haber llamado antes a establer_conexion() ó aceptar conexion
	//Antes de usar enviar_datos()
	if( comm == NULL )
		return -1;
	if( datos == NULL )
		return -2;
	if( dim < 0 )
		return -3;

	//printf("antes del strncpy\n");//DEBUG
	
	memcpy( comm->mesgPtr->data, datos, dim);
	//printf("==>enviar_datos(): datos=%d\n", ((int*)datos)[2*11+2] );
	//printf("==>enviar_datos(): shared mem=%d\n", ((int*)comm->mesgPtr->data)[2*11+2] );
	
	if( comm->tipoComm == CLIENT )
		comm->mesgPtr->id = comm->pid_client;
	else
		comm->mesgPtr->id = comm->pid_server;

	comm->mesgPtr->len = dim;
	//printf("antes del sem_up(semServer)\n");//DEBUG
	//printf("antes del sem_up(semSClient)\n");//DEBUG
	/*
	if( comm->tipoComm == CLIENT )
	{
		//Dejo la zona critica y despierto al servidor				
		sem_up(comm->semServer);
		sem_down(comm->semClient);
		printf("enviar_datos(): process pid=%d SALI de la zona critica\n", comm->pid_client);//DEBUG
	}
	else
	{
		//Dejo la zona critica y despierto al cliente		
		sem_up(comm->semClient);
		sem_down(comm->semServer);
		printf("enviar_datos(): process pid=%d SALI de la zona critica\n", comm->pid_server);//DEBUG
	}
	*///Dejo la zona critica y despierto al servidor				
	sem_up(comm->semServer);
	//printf("enviar_datos(): UP del sem del SERVIDOR\n");//DEBUG
	sem_down(comm->semClient);
	//printf("enviar_datos(): DOWN del sem del CLIENTE\n");//DEBU
	sem_up(comm->semClient);
	//printf("enviar_datos(): UP AGAIN del sem del CLIENTE\n");//DEBUG
	//printf("enviar_datos(): process pid=%d SALI de la zona critica\n", comm->pid_client);//DEBUG
	return 0;
}

int
recibir_datos( commT comm, void *datos, int lim)
{
	//printf("recibir_datos(): entre a RECIBIR DATOS\n");//DEBUG
	//Asumo que antes ya consiguio acceso a la zona critica
	//Obligatorio haber llamado antes a establer_conexion() ó aceptar conexion
	//Antes de usar enviar_datos()
	int dim;
	if( comm == NULL )
			return -1;
	if( datos == NULL )
		return -2;
	if( lim < 0 )
		return -3;

	if( comm->mesgPtr->len < 0)
		fatal("Error al leer datos de shm del servidor\n");

	if( comm->mesgPtr->len > lim )
		return -4;

	//Escribo la nueva informacion que esta en la shared mem
	dim = comm->mesgPtr->len ;
	memcpy( datos, comm->mesgPtr->data, dim);
	//printf("==>enviar_datos(): shared mem=%d\n", ((int*)comm->mesgPtr->data)[2*11+2] );
	//printf("recibir_datos(): datos[2][2]=%d\n", ((int**)datos)[2][2] );

	//Reseteo los flags de la shared mem para indicar que ya se leyeron datos
	comm->mesgPtr->len = 0;
	comm->mesgPtr->id = -1;
	
	//Libero el recurso y salgo de la zona critica
	//printf("antes del sem_up(semServer)\n");//DEBUG
	//printf("antes del sem_up(semSClient)\n");//DEBUG
	/*
	if( comm->tipoComm == CLIENT )
	{
		//Dejo la zona critica y despierto al servidor
		sem_up(comm->semServer);
		sem_down(comm->semClient);
		printf("recibir_datos(): process pid=%d SALI de la zona critica\n", comm->pid_client);//DEBUG
	}
	else
	{
		//Dejo la zona critica y me bloqueo para dejar libre el shared mem
		sem_up(comm->semClient);
		sem_down(comm->semServer);
		printf("recibir_datos(): process pid=%d SALI de la zona critica\n", comm->pid_server);//DEBUG
	}
	*/	
	//Dejo la zona critica y me bloqueo para dejar libre el shared mem
	sem_up(comm->semClient);
	//printf("recibir_datos(): UP del sem del CLIENTE\n");//DEBUG
	//sem_down(comm->semServer);
	//printf("recibir_datos(): DOWN del sem del SERVIDOR\n");//DEBUG
	
	//printf("recibir_datos(): process pid=%d SALI de la zona critica\n", comm->pid_server);//DEBUG
	return dim;
}

