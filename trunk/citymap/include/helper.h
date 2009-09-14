/*
 * common.h
 *
 *  Created on: Sep 9, 2009
 *      Author:
 */

#ifndef COMMON_H_
#define COMMON_H_

#include "mapADT.h"

//GENERAL DEFINITIONS
#define TRUE 1
#define FALSE 0

#define FERROR(label, returnValue) 	{\
										fprintf(stderr, label);\
										return returnValue;\
									}
//---------------------------------------------------------------------

//Help structs for simulation
typedef struct{  
  int y;
  int x;
}coorT;

enum{ NORTH, EAST, SOUTH, WEST};
//--------------------------------------

//Common functions for all modules
void fatal(char *s);


#endif /* COMMON_H_ */
