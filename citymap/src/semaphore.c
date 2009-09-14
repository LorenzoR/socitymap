/*
 * semaphore.c
 *
 *  Created on: Sep 11, 2009
 *      Author:
 */

#include <stdio.h>
#include <stdlib.h>
#include "semaphore.h"

int
sem_init( key_t semkey, int initval )
{
	int status = 0, semid;

	if( ( semid = semget(semkey, 1, SEMPERM|IPC_CREAT|IPC_EXCL) ) == -1 )
	{
		if( errno == EEXIST )
		{
			semid = semget( semkey, 1, 0 );
			//DEBUG
			semun arg;
			arg.val = 0;
			status = semctl(semid, 0 , GETVAL , arg );
			//printf("sem_init(): semaforo ya exitia con valor %d\n", status);
			//END DEBUG
		}
	}
	else
	{
		semun arg;
		arg.val = initval;
		status = semctl(semid, 0 , SETVAL , arg );
	}
	if( semid == -1 || status == -1 )
	{
		perror("initsem failed\n");
		return -1;
	}
	return semid;
}

int
sem_down( int semid )
{
	struct sembuf down_buf;

	down_buf.sem_num = 0;
	down_buf.sem_op = -1;
	down_buf.sem_flg = SEM_UNDO;

	if( semop( semid, &down_buf, 1 ) == -1 )
	{
		printf("semid=%d\n", semid);
		perror( "semaphore down (semid) failed");
		exit(1);
	}
	return 0;
}

int
sem_up( int semid )
{
	struct sembuf up_buf;

	up_buf.sem_num = 0;
	up_buf.sem_op =  1;
	up_buf.sem_flg = SEM_UNDO;

	if( semop( semid, &up_buf, 1 ) == -1 )
	{
		perror( "semaphore up (semid) failed");
		exit(1);
	}
	return 0;
}

int
sem_rm( int semid )
{
	semun arg;
	arg.val = 0;
	if( semctl(semid, 0, IPC_RMID, arg) < 0 )
	{
		//perror( "coudl not remove (semid) !!");
		exit(1);
	}
	return 0;
}


