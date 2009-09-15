/*
 * application.h
 *
 *  Created on: Sep 8, 2009
 *      Author:
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

//Our includes
#include "mapADT.h"
#include "application.h"
#include "session.h"
#include "helper.h"
#include "semaforo.h"
#include "linea.h"

//---------------------------------------------------------------------------
//DEFINITIONS FOR THE SCREEN MAP MADE IN NCURSES

//Origen de coordenadas para la ventana del mapa
#define ORIGIN_X 	0
#define ORIGIN_Y 	0

//number of blocks in X an in Y
#define BLOCKS_Y 6
#define BLOCKS_X 6

//dimensions for the blocks
#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 5

//separation among blocks
#define STREET_SIZE_Y 1
#define STREET_SIZE_X 1

//screen dimensions
#define SCREEN_SIZE_Y   BLOCK_HEIGHT*BLOCKS_Y+STREET_SIZE_Y*(BLOCKS_Y-1)
#define SCREEN_SIZE_X   BLOCK_WIDTH*BLOCKS_X+STREET_SIZE_X*(BLOCKS_X-1)

//Windows size
#define MAP_WINDOW_HEIGHT 39
#define MAP_WINDOW_WIDTH  42

#define LOG_WINDOW_HEIGHT 39
#define LOG_WINDOW_WIDTH  38+30

//Defines minimum size for term screen.(IBM-VGA)
#define	MIN_ROWS	MAP_WINDOW_HEIGHT
#define	MIN_COLS 	MAP_WINDOW_WIDTH+LOG_WINDOW_WIDTH

//Heart beat time for the traffic manager ( microseconds )
#define TIMESTEP 500000 

//-------------------------------------------------------------------------

//Structure to store all data needed fot the main application
typedef struct{
	struct mapCDT *map;	
	WINDOW *mainWindow;
	WINDOW *mapWindowFrame;
	WINDOW *mapWindow;
	WINDOW *logWindowFrame;
	WINDOW *logWindow;
}systemT;


#endif /* APPLICATION_H_ */
