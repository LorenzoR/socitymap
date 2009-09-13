/*
 * application.h
 *
 *  Created on: Sep 8, 2009
 *      Author:
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <curses.h>

//THIS SHOULD BE MAPADT.H
#include "common.h"

typedef struct{
	struct mapCDT *map;	
	WINDOW *mainWindow;
	WINDOW *mapWindow;
}systemT;



#endif /* APPLICATION_H_ */
