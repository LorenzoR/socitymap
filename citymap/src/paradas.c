/*
 * ==================================================
 * filename: parada.c
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
#include "paradas.h"
#include "colectivo.h"
#include "helper.h"





typedef struct paradaT{
	//Position in the map
	coor pos;
	//if people > 0 , the bus have to stop
	peopleADT people;
	paradaADT sig;
}paradaT;

typedef struct peopleT{
	coor pos; /*destiny*/
	peopleADT sig;
	int name;
}peopleT;


int peoplename = 0;


paradaADT newparadas()
{
	return NULL;

}


paradaADT insertParada(paradaADT list, coor pos) {
	paradaADT aux;

		aux = list;
		if (!(list = malloc(sizeof(struct paradaT))))
			fatal("No memory for another bus stop\n");
		list->pos.x = pos.x;
		list->pos.y = pos.y;
		list->people = NULL;
		setState(pos, PARADAVACIO);
		list->sig =aux;


		fprintf(log, "Se crea una parada en x = %d, y  = %d\n" , list->pos.x, list->pos.y);


		return list;

}

int hastostop(paradaADT list, coor pos){
	if( list == NULL)
		return 0;
	if(pos.x == list->pos.x && pos.y == list->pos.y)
		return list->people != NULL;
	return 	hastostop( list->sig, pos);
}


int haspeopletoleave(peopleADT list,  coor pos)
{
	if( list == NULL)
		return 0;
	if(pos.x == list->pos.x && pos.y == list->pos.y)
		return 1;
	return 	haspeopletoleave( list->sig, pos);
}

void removepeople(peopleADT *list,  coor pos)
{
	peopleADT aux;
	if ((list) == NULL)
		return;
	if ((*list)->pos.x == pos.x && (*list)->pos.y == pos.y )
	{
		fprintf(log, "Se baja la persona  %d\n" , (*list)->name);

		/*hay que borrar*/

		aux = (*list)->sig;
		free(*list);
		*list = aux;
	}
	removepeople (&((*list)->sig), pos) ;

}

void  movepeople(paradaADT list,  coor pos, peopleADT  * bus)
{
	peopleADT aux, pre;
	if ((list) == NULL)
		return;
	if (list->pos.x == pos.x && list->pos.y == pos.y )
	{
		fprintf(log, "Se sube la gente de la parada en x= %d, y= %d\n" , list->pos.x, list->pos.y);
		aux = *bus;
		pre = aux;
		if( *bus == NULL)
		{
			*bus = list->people;
		}
		else
		{
			while (aux != NULL)
			{

				pre = aux;
				aux =aux->sig;
			}
			pre->sig = list->people;
		}
		list->people = NULL;
	}
	else
		movepeople( list->sig,   pos, bus);

}





