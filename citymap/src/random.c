/*
 * random.c
 *
 *  Created on: Sep 15, 2009
 *      Author: dazar
 */


/*
 * System includes
 */
#include <stdlib.h>
#include <time.h>

/*
 * Project includes
 */


void iniciarRandom( void ){

	srand(time(NULL));
}

int randoint(int min, int max){

	return min + (int)((rand()/(RAND_MAX +1.0))*(max - min));
}
