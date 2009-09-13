/*
 * sharedMem.h
 *
 *  Created on: Sep 11, 2009
 *      Author:
 */

#ifndef SHAREDMEM_H_
#define SHAREDMEM_H_


void *getMem( key_t key, int size );
int freeMem( void *memPtr );
void freeAllShm(void);


#endif /* SHAREDMEM_H_ */
