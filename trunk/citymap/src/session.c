/*
 * session_fifo.c
 *
 *  Created on: Sep 8, 2009
 *      Author:
 */
#include "session.h"
#include "helper.h"


//Libraries to use FIFOs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

//Global struct to store session info
char buf[MAX_BUFFER_SIZE];
char logBuf[MAX_BUFFER_SIZE];
commT comm;
commT logComm;

int
getNewSession( int tipoComm )
{
	if( (comm=iniciarComm( tipoComm, MAP_SERVER_NAME ) ) == NULL )
		return 1;

	if( (logComm=iniciarComm( tipoComm, LOG_SERVER_NAME ) ) == NULL )
		return 2;
	return 0;
}

int
getMapUpdates(struct mapCDT **mapa )
{
	int n;
	if( mapa == NULL )
		return 1;

	if( comm == NULL )
		return 2;

	if( aceptar_conexion( comm ) == 0 )
			return 3;
	/*
	if( establecer_conexion( session.comm ) == 0 )
		return 3;
	printf("==>ACEPTO conexion\n");//DEBUG
	*/

	if( (n=recibir_datos( comm, (void *)buf, MAX_BUFFER_SIZE )) < 0 )
		return 4;

	*mapa = calloc(1,sizeof(struct mapCDT));

	memcpy( (*mapa), buf, n );


	//memcpy( (*map)->matrix, buf, sizeof((*map)->matrix) );
	//(*map)->dim =MAP_SIZE;
	//printf("RECEIVED: %d size:%d\n", (*map)->matrix[2][2], sizeof((*map)->matrix));//DEBUG
	return 0;
}

int
putMapUpdates(struct mapCDT *map )
{

	if( map == NULL )
			return 1;

	if( comm == NULL )
			return 2;
		//sleep(5);
	/*
	if( aceptar_conexion( session.comm ) == 0 )
			return 3;
	*/
	//printf("entre a putMapUpdates()\n");
	if( establecer_conexion( comm ) == 0 )
		return 3;
		//sleep(5);

	//printf("SEND: %d\n", map->matrix[2][2]);//DEBUG
	memcpy( buf, map, sizeof(struct mapCDT) );
	if( enviar_datos( comm, (void*)buf, sizeof(struct mapCDT)) < 0 )
		return 4;
	return 0;
}







int
getLogUpdates( char **logEntry )
{
	int n;
	if( logEntry == NULL )
		return 1;

	if( logComm == NULL )
		return 2;

	if( aceptar_conexion( logComm ) == 0 )
			return 3;
	/*
	if( establecer_conexion( session.comm ) == 0 )
		return 3;
	printf("==>ACEPTO conexion\n");//DEBUG
	*/

	if( (n=recibir_datos( logComm, (void *)logBuf, MAX_BUFFER_SIZE )) < 0 )
		return 4;

	*logEntry=calloc(1,n+1);

	memcpy( *logEntry, logBuf, n );
	//memcpy( (*map)->matrix, buf, sizeof((*map)->matrix) );
	//(*map)->dim =MAP_SIZE;
	//printf("RECEIVED: %d size:%d\n", (*map)->matrix[2][2], sizeof((*map)->matrix));//DEBUG
	return 0;
}

int
putLogUpdates( char *logEntry )
{
	int len;

	if( logEntry == NULL )
			return 1;

	if( logComm == NULL )
			return 2;
	if( (len = strlen(logEntry))< 0 )
			return 3;
	/*
	if( aceptar_conexion( session.comm ) == 0 )
			return 3;
	*/
	//printf("entre a putMapUpdates()\n");
	if( establecer_conexion( logComm ) == 0 )
		return 3;

	//printf("SEND: %d\n", map->matrix[2][2]);//DEBUG
	memcpy( logBuf, logEntry,len );
	if( enviar_datos( logComm, (void*)logBuf, len) < 0 )
		return 4;
	return 0;
}


int
closeSession( void )
{
	if( comm != NULL )
		cerrarComm( comm );

	if( logComm != NULL )
		cerrarComm( logComm );
	return 0;
}




