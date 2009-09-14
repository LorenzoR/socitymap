#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "helper.h"
#include "comm.h"

//Para el uso de funciones como:  open(), lockf()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Para el uso de la funcion: getpid()
#include <sys/types.h>
#include <unistd.h>

//Para el uso de funciones como : signal()
#include <signal.h>

//Definiciones para la comunicacion via fifo
#define PATH "/tmp/"

#define FULL_PATH_LEN 100
#define CMD_FIFO "cmd"
#define DATA_FIFO "data"

//Definicion del maximo tiempo de espera, si no hay respuesta, entonces se ha
//perdido comunicacion con el otro extremo
#define TIMEOUT 4

//Flags usados para esperar a que llegue un signal
int alarmFlag = FALSE;


//Estructura para los datos fundamentales para realizar comunicaciones
struct comm{
	pid_t pid_server;
	pid_t pid_client;
	int tipoComm;

	//Usando FIFOs
	int fd_command;
	int fd_data;

	char nombreServidor[FULL_PATH_LEN+1];
	char command[FULL_PATH_LEN+1];
	char data[FULL_PATH_LEN+1];
};

//Funciones estaticas auxiliares
static void buildPathFileName( char *path, char *filename, int pid);

//Funciones para manejo de problemas con FIFOs
static void chauServer(int sig);
static void chauClient(int sig);
static void alarmHandler( int sig );


//IMPLEMENTACION DE FUNCIONES PUBLICAS
//====================================

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

	comm->fd_command = -1;
	comm->fd_data = -1;

	comm->pid_client = -1;
	comm->pid_server = -1;

	//Construyo los nombres para los fifos
	buildPathFileName( comm->command, CMD_FIFO, comm->pid_client );
	buildPathFileName( comm->data, DATA_FIFO, comm->pid_client );

	strcpy(comm->nombreServidor, nombreServidor);

	//Creo los ipc
	if ( access(comm->command, 0) == -1 && mknod(comm->command, S_IFIFO|0666, 0) == -1 )
	{
		fatal("Error mknod command");
		free(comm);
	}
	if ( access(comm->data, 0) == -1 && mknod(comm->data, S_IFIFO|0666, 0) == -1 )
	{
		free(comm);
		fatal("Error mknod data");
	}

	if( tipoComm == CLIENT )
	{
		//Obtengo el pid del proceso cliente y guardo el nombre del servidor
		comm->pid_client = getpid();

		//Capturo si se cierra el ipc
		signal(SIGPIPE, chauClient);
	}
	else //ES EL SERVER
	{
		//Obtengo el pid del proceso servidor
		comm->pid_server = getpid();

		signal(SIGPIPE, chauServer);
	}
	return comm;
}

int
cerrarComm( commT comm )
{
	if( comm != NULL )
	{
		liberar_conexion( comm );
		unlink( comm->command );
		unlink( comm->data );
		free( comm );
	}
	return 0;
}

int
establecer_conexion( commT comm )
{
	//Capturo si se cierra el ipc
	signal(SIGPIPE, chauClient);

	//comm->fd_command = open(comm->data, O_RDONLY);

	//Pongo un timer para esperar que abran el IPC del lectura del otro lado
	signal(SIGALRM, alarmHandler);
	alarm(TIMEOUT);
	alarmFlag = FALSE;
	while( (comm->fd_command = open(comm->command, O_WRONLY|O_NONBLOCK) ) == -1 )	
	{
		if( alarmFlag )
		{
			alarm(0);
			signal(SIGALRM, SIG_IGN);
			liberar_conexion( comm );
			return 0;
		}
	}
	return comm->pid_server;
}

int
aceptar_conexion( commT comm )
{
	signal(SIGPIPE, chauServer);

	comm->fd_command = open(comm->command, O_RDONLY);	
	//printf("aceptar_conexion(): se abrio ipc COMMAND para atender cliente\n"); //DEBUG
	/*
	//Pongo un timeout para abrir ipc de una conexion
	signal(SIGALRM, alarmHandler);
	alarm(TIMEOUT);
	alarmFlag = FALSE;
	while( ( comm->fd_data = open(comm->data, O_WRONLY|O_NONBLOCK) ) == -1 )
	{
		if( alarmFlag )
		{
			alarm(0);
			signal(SIGALRM, SIG_IGN);
			liberar_conexion( comm );
			return 0;
		}
	}
	*/
	//printf("aceptar_conexion(): se abrio ipc DATA para atender cliente\n"); //DEBUG
	return comm->pid_client;
}

int
liberar_conexion( commT comm )
{

	if( comm->fd_command != -1 )
	{
		close( comm->fd_command );
		comm->fd_command = -1;
	}
	if( comm->fd_data != -1 )
	{
		close( comm->fd_data );
		comm->fd_data = -1;
	}
	return 0;
}

int
enviar_datos( commT comm, void *datos, int dim)
{
	if( comm->tipoComm == CLIENT )
	{
		if( write(comm->fd_command, datos, dim) != dim )
		{
			//printf("\tSe perdio la conexion con el servidor\n");//DEBUG
			liberar_conexion( comm );
			return -1;
		}
	}
	else
	{
		if( write(comm->fd_data, datos, dim) != dim )
		{
			//printf("enviar_datos(): Error de escritura\n");//DEBUG
			//printf("Se perdio la conexion con el cliente pid %d\n", comm->pid_client);//DEBUG
			liberar_conexion( comm );
			return -comm->pid_client;
		}
	}
	//printf("enviar_datos(): buff=[%s] len=%d\n", (char*)datos, dim); //DEBUG
	return 0;
}


int
recibir_datos( commT comm, void *datos, int lim)
{
	int n;

	if( comm->tipoComm == CLIENT )
		n = read(comm->fd_data, datos, lim);
	else
		n = read(comm->fd_command, datos, lim);

	if( n <= 0 )
	{
		if( comm->tipoComm == CLIENT )
		{
			//printf("recibir_datos(): Error de lectura\n");//DEBUG
			//printf("\tSe perdio la conexion con el servidor\n");//DEBUG
			liberar_conexion( comm );
			return -1;
		}
		else
		{
			//printf("recibir_datos(): Error de lectura\n");//DEBUG
			printf("Se perdio la conexion con el cliente pid %d\n", comm->pid_client);
			liberar_conexion( comm );
			return -comm->pid_client;
		}
	}
	//printf("recibir_datos(): buff=[%s] len=%d\n", (char*)datos, n); //DEBUG
	return n;
}


//IMPLEMENTACION DE FUNCIONES ESTATICAS INTERNAS
//==============================================

//Implementacion de funciones estaticas auxiliares
static void
buildPathFileName( char *path, char *filename, int pid)
{
	//char cad_pid[100+1];

	path[0] = 0;
	strcat( path, PATH );
	strcat( path, filename );
	/*
	if( pid > 0 )
	{
		sprintf(cad_pid, "%d", pid);
		strcat( path, cad_pid);
	}
	*/
	//printf("buildPathFIleName(): path=[%s]\n", path);//DEBUG
	return;
}
static void
chauServer(int sig)
{
	printf("Servidor: Se recibio SIGPIPE %d.\n", sig);
	return;
}

static void
chauClient(int sig)
{
	printf("Cliente: Se recibio SIGPIPE %d.\n", sig);
	return;
}
static void
alarmHandler( int sig )
{
	alarmFlag = TRUE;
	return;
}
