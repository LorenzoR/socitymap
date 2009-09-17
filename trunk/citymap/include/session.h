/*
 * session_fifo.h
 *
 *  Created on: Sep 8, 2009
 *      Author:
 */

#ifndef SESSION_FIFO_H_
#define SESSION_FIFO_H_

#include "helper.h"
#include "comm.h"

#define MAX_BUFFER_SIZE sizeof(struct mapCDT)//4*MAXX*MAXY*3

int getNewSession(int tipoComm );
int closeSession( void );

int getMapUpdates( struct mapCDT **map );
int putMapUpdates( struct mapCDT *map );

int getLogUpdates( char **logEntry );
int putLogUpdates( char *logEntry );

#endif /* SESSION_FIFO_H_ */
