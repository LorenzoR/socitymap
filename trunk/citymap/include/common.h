/*
 * common.h
 *
 *  Created on: Sep 9, 2009
 *      Author:
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "mapADT.h"

//THIS SHOULD BE AT THE MAP ADT
#define MAP_SIZE 11

typedef struct{
	int matrix[MAP_SIZE][MAP_SIZE];
	int dim;
}mapT;
//----------------------------------

//Help structs for simulation
typedef struct{  
  int y;
  int x;
}coorT;

enum{ NORTH, EAST, SOUTH, WEST};

//--------------------------------------

//GENERAL DEFINITIONS
#define TRUE 1
#define FALSE 0

#define FERROR(label, returnValue) 	{\
										fprintf(stderr, label);\
										return returnValue;\
									}

void fatal(char *s);


#endif /* COMMON_H_ */
