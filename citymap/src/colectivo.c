/*
 * ==================================================
 * filename: colectivo.c
 * Project: Ciudad
 *
 * 		Created on: sep 1, 2009 2:31:0 PM
 * 		Last modified on:
 * 		Author: Daniel Azar
 * ==================================================
 */

/*
 * System includes
 */
#include <stdlib.h>

/*
 * Project includes
 */

#include "mapADT.h"
#include "colectivo.h"

#include "paradas.h"
#include "helper.h"



typedef struct listT{
	colectivoADT bus;
	//route in the map
	coor * route;
	int cantRoute;
	int cantParadas;
	paradaADT paradas;
}listT;



typedef struct colectivoT{
	//Position in the map
	coor pos;
	//segment destination point
	coor NextRoute;
	/*int timestop;//the time to wait to move if it is on a bus stop.*/
	int lastMovedOn;
	colectivoADT sig;
	int dir;
	peopleADT people;
	int name;
}colectivoT;
/*
static int GraphIsEmpty(colectivoADT graph) {
	return graph == NULL;
}
*/
int newname = 0;




void busGeneratePeople(listADT bus){

	paradaGeneratePeople(bus->paradas, bus->cantParadas);
}



static void getNewRoute(colectivoADT bus, listADT list)
{
	int i = 0;
	while (i< list->cantRoute)
	{
		//fprintf(log, "la pos del colectivo es %d, %d\n", bus->pos.x , bus->pos.y);
		//fprintf(log, "el nodo es %d, %d\n", list->route[i].x, list->route[i].y);
		//fprintf(log, "el nodo sig es %d, %d\n", list->route[i+1].x, list->route[i+1].y);

		if (bus->pos.x == list->route[i].x && bus->pos.y == list->route[i].y)
		{
			if (i+1 == list->cantRoute ) /*estoy en el ultimo vertice*/
			{
				bus->NextRoute.x = list->route[0].x;
				bus->NextRoute.y = list->route[0].y;
			}
			else
			{
				bus->NextRoute.x = list->route[i+1].x;
				bus->NextRoute.y = list->route[i+1].y;
			}

			if(bus->pos.x < bus->NextRoute.x)
				bus->dir = ESTE;
			else
				if(bus->pos.x > bus->NextRoute.x)
					bus->dir = OESTE;
				else
					if(bus->pos.y > bus->NextRoute.y)
						bus->dir = SUR;
					else
					if(bus->pos.y < bus->NextRoute.y)
						bus->dir = NORTE;
			break;
		}
		++i;
	}
	//fprintf(log, "la pos del colectivo es %d, %d\n", bus->pos.x , bus->pos.y);
	//fprintf(log, "el nuevo destino del colectivo es %d, %d\n", bus->NextRoute.x, bus->NextRoute.y);
	//fprintf(log, "la dir del colectivo es %d", bus->dir);



}

static int updateBus(colectivoADT  bus,int  time, listADT list)
{
	coor pos;
	if(bus == NULL)
		return NOTCHANGE;
	if (iAmOnBusStop(bus->pos))
	{
		if (haspeopletoleave(bus->people ,bus->pos))
		{
			fprintf(log, "Se bajan personas del micro %d\n" , bus->name);
			removepeople(&(bus->people), bus->pos);
			bus->lastMovedOn = time;
		}
		if (hastostop(list->paradas ,bus->pos))
		{
			fprintf(log, "va a subir gente al micro %d\n" , bus->name);
			movepeople(list->paradas ,bus->pos , &(bus->people));
			bus->lastMovedOn = time;
		}
	}
	if(time != bus->lastMovedOn)
	{

		switch (bus->dir)
		{
			case NORTE:
				pos.x = bus->pos.x;
				pos.y = bus->pos.y + 1;
				break;
			case OESTE:
				pos.x = bus->pos.x - 1;
				pos.y = bus->pos.y;
				break;
			case ESTE:
				pos.x = bus->pos.x;
				pos.y = bus->pos.y + 1;
				break;
			case SUR:
				pos.x = bus->pos.x;
				pos.y = bus->pos.y - 1;
				break;

		}

		if (canMove(pos, bus->dir))
		{

			setState(bus->pos , getState(bus->pos )-1);/*pasa de lleno a vacio*/
			setName(bus->pos, -1 );
			bus->pos.x = pos.x;
			bus->pos.y = pos.y;
			setState(bus->pos , getState(bus->pos) + 1); /*pasa de vacio a lleno*/
			setName(bus->pos, bus->name);
			bus->lastMovedOn = time;

			getNewRoute(bus, list);
			return CHANGE + updateBus(bus->sig, time, list);
		}
	}


	return NOTCHANGE + updateBus(bus->sig, time , list);
}

listADT newBuses(coor route[], int cant, paradaADT paradas, int cantParadas)
{
	listADT aux;
	if (!(aux = malloc(sizeof(struct listT))))
		fatal("No memory for buses list\n");
		aux->cantRoute =cant;
		aux->route = route;
		aux->bus = NULL;
		aux->paradas = paradas;
		aux->cantParadas = cantParadas;
		return aux;

}


int updeteColectivos(listADT list,int  time)
{
	return updateBus(list->bus, time, list);
}

void InsertBus(listADT list, coor pos, int time) {
	colectivoADT aux;

		aux = list->bus;
		if (!(list->bus = malloc(sizeof(struct colectivoT))))
			fatal("No memory for another bus\n");
		list->bus->pos.x = pos.x;
		list->bus->pos.y = pos.y;
		getNewRoute(list->bus, list);
		list->bus->lastMovedOn = time;
		list->bus->name = newname;
		list->bus->sig =aux;
		list->bus->people = NULL;

		setState(list->bus->pos , getState(list->bus->pos) + 1); /*pasa de vacio a lleno*/
		setName(list->bus->pos , list->bus->name);

		newname++;

		fprintf(log, "Se creo un colectivo %d\n" , list->bus->name);


}





