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
#include "session.h"


typedef struct listT{
	colectivoADT bus;
	//route in the map
	coor * route;
	int cantRoute;
	int cantParadas;
	paradaADT paradas;
	int name;
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
int newname = 0, newLineName = 0;




void busGeneratePeople(listADT bus){

	paradaGeneratePeople(bus->paradas, bus->cantParadas);
}



static void getNewRoute(colectivoADT bus, listADT list)
{
	int i = 0;
	//fprintf(log, "el colectivo %d esta en %d, %d\n" , list->bus->name, list->bus->pos.x , list->bus->pos.y);
	//fprintf(logs, "la proxima ruta es %d, %d\n" , list->bus->NextRoute.x , list->bus->NextRoute.y);

	while (i< list->cantRoute)
	{
	//	fprintf(logs, "la pos del colectivo es %d, %d\n", bus->pos.x , bus->pos.y);
	//	fprintf(logs, "el nodo es %d, %d\n", list->route[i].x, list->route[i].y);
	//	fprintf(logs, "el nodo sig es %d, %d\n", list->route[i+1].x, list->route[i+1].y);

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
	//fprintf(logs, "la pos del colectivo es %d, %d\n", bus->pos.x , bus->pos.y);
	//fprintf(logs, "el nuevo destino del colectivo es %d, %d\n", bus->NextRoute.x, bus->NextRoute.y);
	//fprintf(logs, "la dir del colectivo es %d\n", bus->dir);



}

static int updateBus(colectivoADT  bus,int  time, listADT list)
{
	coor pos;
	char test[100];
	if(bus == NULL)
		return NOTCHANGE;
	if (iAmOnBusStop(bus->pos))
	{
		if (haspeopletoleave(bus->people ,bus->pos))
		{
			sprintf(test ,"Se bajan personas del micro %d\n" , bus->name);
			putLogUpdates( logs );
			removepeople(&(bus->people), bus->pos);
			bus->lastMovedOn = time;
			sprintf(logs, "se bajo gente del micro %d\n" , bus->name);
			putLogUpdates( logs );
		}
		if (hastostop(list->paradas ,bus->pos))
		{
			
			sprintf(logs, "va a subir gente al micro %d\n" , bus->name);
			putLogUpdates( logs );
			movepeople(list->paradas ,bus->pos , &(bus->people));
			bus->lastMovedOn = time;
			sprintf(logs, "subio la gente al micro %d\n" , bus->name);
			putLogUpdates( logs );
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
				pos.x = bus->pos.x + 1;
				pos.y = bus->pos.y;
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
			clearLineName(bus->pos);
			bus->pos.x = pos.x;
			bus->pos.y = pos.y;
			
			setState(bus->pos , getState(bus->pos) + 1); /*pasa de vacio a lleno*/
			setName(bus->pos, bus->name);
			setLineName(bus->pos , list->name);
			bus->lastMovedOn = time;
			//sprintf(logs, "colectivo linea %d\n" , getLineName(bus->pos));
			//putLogUpdates( logs );
			
			getNewRoute(bus, list);
			return CHANGE + updateBus(bus->sig, time, list);
		}
	}


	return NOTCHANGE + updateBus(bus->sig, time , list);
}

listADT newBuses(coor route[], int cant, paradaADT paradas, int cantParadas, char * name)
{
	listADT aux;
	if (!(aux = malloc(sizeof(struct listT))))
		fatal("No memory for buses list\n");
		aux->cantRoute =cant;
		aux->route = route;
		aux->bus = NULL;
		aux->paradas = paradas;
		aux->cantParadas = cantParadas;
		aux->name = newLineName;
		sprintf(logs, "El pseudo nombre de la linea %s, es %d\n" , name, aux->name);
		putLogUpdates( logs );
		newLineName++;
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
		setLineName(list->bus->pos , list->name);	
		
		newname++;

		sprintf(logs, "Se creo un colectivo %d\n" , list->bus->name);
		putLogUpdates( logs );

}






