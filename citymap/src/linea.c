/*
 * ==================================================
 * filename: lineas.c
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

/*
 * Project includes
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mapADT.h"
#include "linea.h"
#include "helper.h"
#include "colectivo.h"
#include "session.h"



typedef struct lineaT{

	//position of the start of the line
	coor start;
	int timeToStart;//the time to to send another bus.
	int busesToSend;
	int busesSended;
	listADT  buses;
	char name[30];

}lineaT;

/*
lineaADT newlinea(coor route[], int cant, paradaADT paradas, int busesToSend, coor start ,int timeToStart)
{

	aux->buses = newBuses( route, cant,  paradas);
	aux->busesSended = 0;
	aux->busesToSend = busesToSend;
	aux->timeToStart = timeToStart;
	aux->start.x = start.x;
	aux->start.y = start.y;
	return aux;
}
*/

int updateLinea (lineaT * line, int time)
{
  int change = 0;

  FILE *logs;
  	logs = fopen("log", "a+");
 // fprintf(logs, "busesSended %d , busesToSend %d, time %d , timeToStart %d",line->busesSended, line->busesToSend, time ,line->timeToStart);
  if (line->busesSended < line->busesToSend && ((time %  line->timeToStart == 0 && (line->busesSended -1)  == time / line->timeToStart )||  (line->busesSended -1)  < time / line->timeToStart))
   {
	//  fprintf(logs , "esta vacio ? %d \n",isSpaceEmpty(line->start));
      if (isSpaceEmpty(line->start))
      {
    	//fprintf(logs, "Se crea un colectivo en la linea %s\n" , line->name);
    	//putLogUpdates( logs );
		InsertBus(line->buses, line->start, time);
		line->busesSended++;
		change = 1;
      }
   }

       if (updeteColectivos( (line->buses) , time)  > NOTCHANGE )
	  		change = 1;


       fclose(logs);
   return change;
}


void generatePeople(lineaADT linea )
{
	FILE *logs;
	logs = fopen("log", "a+");

	//fprintf(logs , "se crea una persona en la linea %s\n", linea->name);
	//putLogUpdates( logs );
	busGeneratePeople(linea->buses);
	//fprintf(logs , "se creo la persona en la linea %s\n", linea->name);
	//putLogUpdates( logs );
	fclose(logs);
}


lineaADT ReadBusLine(char * arch){
	lineaADT aux;
	paradaADT paux;
	FILE * fd;
	char archivo[100];
	//archivo[99] = '\0';
	int cantparada , cantruta, i;
	coor pos, *ruta;
	FILE *logs;
	logs = fopen("log", "a+");

	if ((aux =	malloc (sizeof(lineaT)))==NULL)
	  	fatal("Error allocating memory for bus line");
	strcpy(aux->name, arch);
	strcpy( archivo, DIRLINEA);
	strncat (archivo, arch, 30);
	//archivo[99] = '\0';
	//printf("%s\n", archivo);


	//printf("aaaa");
	if ((fd = fopen(archivo,"rt")) == NULL)
	{
		fatal("In open of bus line configuration file:");
	}

	strcpy(aux->name, arch);
	//printf("%s\n", aux->name);
	fscanf(fd, "%d %d\n", &aux->busesToSend, &aux->timeToStart);

	fscanf(fd, "%d %d\n", &aux->start.x, &aux->start.y);

	/*aca tendria que haber una lista de paradas
	 *
	 * y despues una de coordenada*/
	paux = newparadas();
	fscanf(fd, "paradas %d\n", &cantparada);
	//fprintf(logs, "cant de paradas %d\n" , cantparada);
	i = cantparada;
	while(i != 0)
	{
		//fprintf(logs, "se crea una parada en la linea %s\n" , aux->name);
		//putLogUpdates( logs );
		fscanf(fd, "%d %d\n", &pos.x, &pos.y);
		paux = insertParada(paux, pos);
		--i;

	}

	fscanf(fd, "ruta %d\n", &cantruta);

	if ((ruta =	malloc (sizeof(coor) * cantruta))==NULL)
		  	fatal("Error allocating memory for bus route");

	for(i= 0 ; i < cantruta ; ++i)
		{
			fscanf(fd, "%d %d\n", &ruta[i].x, &ruta[i].y);
		}



	aux->buses = newBuses( ruta, cantruta,  paux, cantparada, aux->name);
	aux->busesSended = 0;

	/*Comprobacion*/

	fprintf(logs, "Se cargo la linea %s\n" , aux->name);
	//putLogUpdates( logs );
	fprintf(logs, "nombe de archivo  %s\n" , aux->name);
	//putLogUpdates( logs );
	fprintf(logs, "buses a enviar  %d\n" , aux->busesToSend);
	//putLogUpdates( logs );
	fprintf(logs, "Tiempo a esperar  %d\n" , aux->timeToStart);
	//putLogUpdates( logs );
	fprintf(logs, "empieza en x %d, y %d\n" , aux->start.x, aux->start.y);
	//putLogUpdates( logs );
	fprintf(logs, "cant de paradas %d\n" , cantparada);
	//putLogUpdates( logs );

	if (fclose(fd) == EOF)
		{
			fatal("In close of trafic lights  configuration file:");
		}
	return aux;
	fclose(logs);
	}
