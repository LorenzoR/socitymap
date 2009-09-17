/*
 * ==================================================
 * filename: mapADT.h
 * Project: ciudad
 *
 * 		Created on: sep 4, 2009 4:30:0 AM
 * 		Last modified on:
 * 		Author: Daniel Azar
 * ==================================================
 */
#ifndef MAPADT_H_
#define MAPADT_H_

/*para probar*/
#include <stdio.h>
//char logs[200];
FILE *logs;
/*hasta aca*/


enum {NORTE =1, OESTE=2 , ESTE=3, SUR=4};
enum {VACIO = 0, LLENO = 1, VERDEVERTICALVACIO = 2, VERDEVERTICALLLENO = 3, ROJOVERTICALVACIO = 4, ROJOVERTICALLENO =5,  PARADAVACIO = 6, PARADASLLENO = 7};

#define MAXX 17
#define MAXY MAXX
#define MAP_SIZE 4*MAXX*MAXY

typedef struct coor{
//Position in the map
  int x;
  int y;
}coor;

struct mapCDT
{
	int state[MAXY][MAXX];
	int name[MAXY][MAXX];
	int linename[MAXY][MAXX];
};


struct mapCDT map;

int getName(coor pos);
void setName(coor pos, int name);
void setState(coor pos, int dato);
int getState(coor pos) ;
int canMove(coor pos, int dir);
int isSpaceEmpty(coor p);
int iAmOnBusStop(coor p);

void setLineName(coor pos ,int name);
int getLineName(coor pos);
void clearLineName(coor pos);



int getLineName2(struct mapCDT mapa,coor pos);
int getState2(struct mapCDT mapa,coor pos);
int getName2(struct mapCDT mapa,coor pos);


#endif /* MAPADT_H_ */
