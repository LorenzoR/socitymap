/*
 * ==================================================
 * filename: semaforo.c
 * Project: Ciudad
 *
 * 		Created on: sep 1, 2009 2:26:0 PM
 * 		Last modified on:
 * 		Author: Daniel Azar
 * ==================================================
 */

/*
 * System includes
 */

#include <stdio.h>
#include <stdlib.h>
/*
 * Project includes
 */

#include "mapADT.h"
#include "semaforo.h"
#include "helper.h"
#include "session.h"

/*
 * Macros and definitions
 */



typedef struct semaforoT{
	//Position in the map
	coor pos;
	//Direction facing
	int state; //NS or EW
	int timechange;//the time to wait tochange the state.
}semaforoT;

enum {VERDEVERT, ROJOVERT};

semaforoADT traficLight = NULL;
int cantLights;


static int verifysem(semaforoT sem)
{
	return sem.pos.x > 0 && sem.pos.x < MAXX &&	sem.pos.y > 0 && sem.pos.y < MAXY ;
}

void newLights(void)
{
	FILE * arch;
	int i, count = 0;
	semaforoT aux;

	FILE *logs;
	logs = fopen("log", "a+");
	/**************************************************************************
	 * loading of traffic lights from file.
	 * ************************************************************************/
	if ((arch = fopen(ARCHSEMAFORO,"rt")) == NULL)
	{
		fatal("In open of trafic lights configuration file:");
	}
/*creo un archivo de logs para probar*/
/*	if ((logs = fopen("logsFile","wt")) == NULL)
		{
			fatal("In open logs File:");
		}*/
	/*creo un archivo de logs para probar*/


	while ((i = fscanf(arch, "%d %d %d\n", &aux.pos.x, &aux.pos.y, &aux.timechange))
			!= EOF)
	{
		if (i != 3)
			fprintf(stderr, "Invalid trafic light in file\n");

		else
			if( verifysem(aux))
			{
				++count;
				traficLight =	realloc (traficLight, count * sizeof(semaforoT));
				 if (traficLight==NULL)
  		     		fatal("Error (re)allocating memory");
   		    	traficLight[count-1].pos.x =  aux.pos.x;
  		     	traficLight[count-1].pos.y = aux.pos.y;
    		   	traficLight[count-1].timechange = aux.timechange;
       			traficLight[count-1].state = VERDEVERT;
       			setState(aux.pos, VERDEVERTICALVACIO);
       			//fprintf(logs, "Se creo un semaforo en x= %d, y= %d, tiempo de cambio = %d\n",traficLight[count-1].pos.x , traficLight[count-1].pos.y, traficLight[count-1].timechange );
				//putLogUpdates( logs );
			}
    }
	cantLights = count;
	if (fclose(arch) == EOF)
	{
		fatal("In close of trafic lights  configuration file:");
	}
	fclose(logs);
}



static void changeStateSemlight( semaforoT *light)
{
	FILE *logs;
	logs = fopen("log", "a+");
  if( (light->state == VERDEVERT) )
  {
    light->state = ROJOVERT;
    setState(light->pos, getState(light->pos) + ( ROJOVERTICALVACIO - VERDEVERTICALVACIO));
    //fprintf(logs, "el semaforo en x= %d, y= %d, cambio a rojo vertical\n",light->pos.x , light->pos.y);
  	//putLogUpdates( logs );
  }
  else
  {
    light->state = VERDEVERT;
     setState(light->pos, getState(light->pos) - ( ROJOVERTICALVACIO - VERDEVERTICALVACIO));
      //fprintf(logs, "el semaforo en x= %d, y= %d, cambio a verde vertical\n", light->pos.x , light->pos.y);
	//putLogUpdates( logs );
  }
  fclose(logs);
}

void changeSemaforo( int time )
{
int i;
for (i = 0; i < cantLights; ++i)
  {
    if (time % traficLight[i].timechange == 0)
    changeStateSemlight(&traficLight[i]);
  }
}

