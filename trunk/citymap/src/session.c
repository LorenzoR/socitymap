/*
 * session_fifo.c
 *
 *  Created on: Sep 8, 2009
 *      Author:
 */
#include "session.h"
#include "common.h"


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
commT comm;


int
getNewSession( int tipoComm )
{
	if( (comm=iniciarComm( tipoComm, SERVER_NAME ) ) == NULL )
		return 1;	

	return 0;
}

int
getMapUpdates(struct mapCDT **map )
{
	int n;
	if( map == NULL )
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
	
	*map = malloc(sizeof(struct mapCDT));
	memcpy( (*map)->state, buf, n );
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
	/*	
	if( aceptar_conexion( session.comm ) == 0 )
			return 3;
	*/
	//printf("entre a putMapUpdates()\n");
	if( establecer_conexion( comm ) == 0 )
		return 3;
		
	//printf("SEND: %d\n", map->matrix[2][2]);//DEBUG
	memcpy( buf, map->state, sizeof(map->state) );
	if( enviar_datos( comm, (void*)buf, sizeof(map->state)) < 0 )
		return 4;
	return 0;
}

int
closeSession( void )
{
	if( comm != NULL )
		cerrarComm( comm );
	
	return 0;
}




