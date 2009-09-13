/*
 * session_fifo.h
 *
 *  Created on: Sep 8, 2009
 *      Author:
 */

#ifndef SESSION_FIFO_H_
#define SESSION_FIFO_H_

#include "common.h"
#include "comm.h"

#define MAX_BUFFER_SIZE 512
/*
typedef struct{
	char buf[MAX_BUFFER_SIZE];
	commT comm;
}sessionT;
*/
int getNewSession(int tipoComm );
int getMapUpdates( struct mapCDT **map );
int putMapUpdates( struct mapCDT *map );
int closeSession( void );

#endif /* SESSION_FIFO_H_ */
