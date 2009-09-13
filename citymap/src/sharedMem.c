/*
 * sharedMem.c
 *
 *  Created on: Sep 11, 2009
 *      Author:
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "sharedMem.h"

#define SHMEMPERM 0666
#define MAX_BLOCKS 200

static int shmid_array[MAX_BLOCKS];
static int pos;

void *
getMem( key_t key, int size )
{
	int shmid;
	void * p;

	printf("getMem():entre a crear la shm, key=%X", key);//DEBUG
	if( ( shmid = shmget(key, size, SHMEMPERM|IPC_CREAT|IPC_EXCL) ) == -1 )
	{
		if( ( shmid = shmget(key, size, 0 ) ) == -1 )
			return NULL;
	}
	if( ( p = shmat( shmid, 0, 0 ) ) == (void *)-1 )
	{
		shmctl( shmid, IPC_RMID, NULL );
		return NULL;
	}
	shmid_array[pos++] = shmid;
	printf("getMem():shm en dir=%X", p);//DEBUG

	return p;
}

int
freeMem( void *memPtr )
{
	if( shmdt(memPtr) < 0 )
		return 1;
	return 0;
}

void
freeAllShm(void)
{
	int i;

	for( i=0 ; i< pos ; i++ )
		shmctl( shmid_array[i], IPC_RMID, NULL );
	pos = 0;
	return;
}
