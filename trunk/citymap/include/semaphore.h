/*
 * semaphore.h
 *
 *  Created on: Sep 11, 2009
 *      Author:
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

typedef union _semun{
	int val;
	struct semid_ds *buf;
	ushort *array;
}semun;

int sem_init( key_t semkey, int initval );
int sem_down( int semid );
int sem_up( int semid );
int sem_rm( int semid );


#endif /* SEMAPHORE_H_ */
