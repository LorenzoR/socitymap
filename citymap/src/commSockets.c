#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"
#include "comm.h"

//Para el uso de funciones como:  open(), lockf()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Para el uso de la funcion: getpid()
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

//Definiciones para sockets
#define ADDRESS "localhost"
#define PORT "60000"

//Flags usados para esperar a que llegue un signal
int alarmFlag = FALSE;


//Estructura para los datos fundamentales para realizar comunicaciones
struct comm{
	pid_t pid_server;
	pid_t pid_client;
	int tipoComm;	
	
	// Sockets
	int udpSocket;
	
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

// Funciones de sockets
int connectsock(const char *host, const char *service, const char *transport );
int connectUDP(const char *host, const char *service );
int passivesock(const char *service, const char *transport, int qlen);
int passiveUDP(const char *service);
int errexit(const char *format, ...);

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
	
	comm->udpSocket = -1;

	comm->pid_client = -1;
	comm->pid_server = -1;
			
	//Construyo los nombres para los fifos
	buildPathFileName( comm->command, CMD_FIFO, comm->pid_client );
	buildPathFileName( comm->data, DATA_FIFO, comm->pid_client );
	
	strcpy(comm->nombreServidor, nombreServidor);
	

	if( tipoComm == CLIENT )
	{			
		//Obtengo el pid del proceso cliente y guardo el nombre del servidor 
		comm->pid_client = getpid();		
		
		//Capturo si se cierra el ipc
		signal(SIGPIPE, chauClient);		
	}
	else  
	{	
	      //Es el server, inicializo el socket para escuchar
	      if ( (comm->udpSocket = passiveUDP(PORT) ) == -1 )
	      {
		    free(comm);
		    fatal("Error server socket");
	      }

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
	
	//Pongo un timer para esperar que abran el IPC del lectura del otro lado
	signal(SIGALRM, alarmHandler);
	alarm(TIMEOUT);
	alarmFlag = FALSE;
	
	//Si el socket no esta abierto, lo abro para enviar
	if ( comm->udpSocket == -1 )
	{
	      if ( (comm->udpSocket = connectUDP(ADDRESS, PORT) ) == -1 )
	      {
		      if( alarmFlag )
		      {
			      alarm(0);
			      signal(SIGALRM, SIG_IGN);
			      liberar_conexion( comm );
			      return 0;
		      }		
	      }
	}
	
	return comm->udpSocket;
}

int 
aceptar_conexion( commT comm )
{	
	signal(SIGPIPE, chauServer);
	
	if ( comm->udpSocket == -1 )
	      return 0;
	else
	      return comm->udpSocket;
}

int 
liberar_conexion( commT comm )
{
	
	if( comm->udpSocket != -1 )
	{
		close( comm->udpSocket );
		comm->udpSocket = -1;
	}

	return 0;	
}

int 
enviar_datos( commT comm, void *datos, int dim)
{	 
	int n;

	if( ( n = write(comm->udpSocket, datos, dim) ) != dim )
	{	
		liberar_conexion( comm );
		return -1;
	}
	else
		return n;
}


int 
recibir_datos( commT comm, void *datos, int lim)
{
	int n;
	
	if( ( n = read(comm->udpSocket, datos, lim) ) <= 0 )
	{	
		liberar_conexion( comm );
		return -1;
	}
	else
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

// FUNCIONES DE SOCKET
//==============================================

int
passivesock(const char *service, const char *transport, int qlen)
//
// Arguments:
//      service   - service associated with the desired port
//      transport - transport protocol to use ("tcp" or "udp")
//      qlen      - maximum server request queue length
//
{
	struct servent	*pse;	// pointer to service information entry	
	struct protoent *ppe;	// pointer to protocol information entry
	struct sockaddr_in sin;	// an Internet endpoint address		
	int	s, type;	// socket descriptor and socket type	
	unsigned short	portbase = 0;	/* port base, for non-root servers	*/

	if ( service == NULL || transport == NULL )
		return -1;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

    	// Map service name to port number 
	if ( (pse = getservbyname(service, transport)) )
		sin.sin_port = htons(ntohs((unsigned short)pse->s_port) + portbase);

	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
	{
		fprintf(stderr, "can't get \"%s\" service entry\n", service);
		return -1;
	}

    	// Map protocol name to protocol number 
	if ( (ppe = getprotobyname(transport)) == 0)
	{
		fprintf(stderr,"can't get \"%s\" protocol entry\n", transport);
		return -1;
	}	

    	// Use protocol to choose a socket type 
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    	// Allocate a socket 
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0)
	{
		fprintf(stderr,"can't create socket: %s\n", strerror(errno));
		return -1;
	}

    	// Bind the socket 
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		fprintf(stderr,"can't bind to %s port: %s\n", service, strerror(errno));
		return -1;
	}	

	if (type == SOCK_STREAM && listen(s, qlen) < 0)
	{
		fprintf(stderr,"can't listen on %s port: %s\n", service,strerror(errno));
		return -1;
	}

	return s;
}


//------------------------------------------------------------------------
//passiveUDP - create a passive socket for use in a UDP server
//------------------------------------------------------------------------
//
int
passiveUDP(const char *service)
//
// Arguments:
//     service - service associated with the desired port
//
{
	if ( service == NULL )
		return -1;
	else 
		return passivesock(service, "udp", 0);
}

int
errexit(const char *format, ...)
{
	va_list	args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}


/*------------------------------------------------------------------------
 * connectsock - allocate & connect a socket using TCP or UDP
 *------------------------------------------------------------------------
 */
int
connectsock(const char *host, const char *service, const char *transport )
/*
 * Arguments:
 *      host      - name of host to which connection is desired
 *      service   - service associated with the desired port
 *      transport - name of transport protocol to use ("tcp" or "udp")
 */
{
	struct hostent	*phe;	/* pointer to host information entry	*/
	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/

	if ( host == NULL || service == NULL || transport == NULL )
		return -1;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

    /* Map service name to port number */
	if ( (pse = getservbyname(service, transport)) )
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
	{
		printf("can't get \"%s\" service entry\n", service);
		return -1;
	}
    /* Map host name to IP address, allowing for dotted decimal */
	if ( (phe = gethostbyname(host)) )
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
	{
		printf("can't get \"%s\" host entry\n", host);
		return -1;
	}
    /* Map transport protocol name to protocol number */
	if ( (ppe = getprotobyname(transport)) == 0)
	{
		printf("can't get \"%s\" protocol entry\n", transport);
		return -1;
	}
    /* Use protocol to choose a socket type */
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    /* Allocate a socket */
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0)
	{
		printf("can't create socket: %s\n", strerror(errno));
		return -1;
	}
    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("can't connect to %s.%s: %s\n", host, service,
			strerror(errno));
		return -1;
	}
	return s;
}


/*------------------------------------------------------------------------
 * connectUDP - connect to a specified UDP service on a specified host
 *------------------------------------------------------------------------
 */
int
connectUDP(const char *host, const char *service )
/*
 * Arguments:
 *      host    - name of host to which connection is desired
 *      service - service associated with the desired port
 */
{
	if ( host == NULL || service == NULL )
		return -1;
	else
		return connectsock( host, service, "udp");
}


