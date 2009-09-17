/*
 * ==================================================
 * filename: mapADT.c
 * Project: Ciudad
 *
 * 		Created on: sep 4, 2009 4:30:0 AM
 * 		Last modified on:
 * 		Author: Daniel Azar
 * ==================================================
 */

/*
 * System includes
 */
#include <string.h>

/*
 * Project includes
 */

#include "mapADT.h"



/*
 * Macros and definitions
 */


void setName(coor pos, int name){
	map.name[pos.y][pos.x] = name;
}







int canMove(coor pos, int dir)
{
  return map.state[pos.y][pos.x] == VACIO ||  map.state[pos.y][pos.x] == PARADAVACIO || ((dir == NORTE || dir == SUR )&& map.state[pos.y][pos.x] ==  VERDEVERTICALVACIO) || ((dir == ESTE || dir == OESTE) && map.state[pos.y][pos.x] ==  ROJOVERTICALVACIO )  ;
}


void setState(coor pos, int dato)
{
  map.state[pos.y][pos.x] = dato;
}

int isSpaceEmpty(coor p){

	return map.state[p.y][p.x]	== VACIO || map.state[p.y][p.x]	== PARADAVACIO;
}

int iAmOnBusStop(coor p){

	return map.state[p.y][p.x] == PARADASLLENO ;
}

void setLineName(coor pos ,int name)
{
	map.linename[pos.y][pos.x] = name;
}

void clearLineName(coor pos)
{
	map.linename[pos.y][pos.x] = -1 ;
}


int getName2(struct mapCDT mapa,coor pos){
	return mapa.name[pos.y][pos.x];
}

int getLineName2(struct mapCDT mapa,coor pos)
{

	return 	mapa.linename[pos.y][pos.x];
}
int getState2(struct mapCDT mapa,coor pos)
{
  return mapa.state[pos.y][pos.x];
}


int getName(coor pos){
	return map.name[pos.y][pos.x];
}

int getLineName(coor pos)
{

	return 	map.linename[pos.y][pos.x];
}
int getState(coor pos)
{
  return map.state[pos.y][pos.x];
}
