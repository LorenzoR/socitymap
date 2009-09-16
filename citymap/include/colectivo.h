#ifndef COLECTIVO_H_
#define COLECTIVO_H_
typedef struct listT * listADT;
typedef struct colectivoT * colectivoADT;
#define CHANGE 1
#define NOTCHANGE 0
#include "paradas.h"
void InsertBus(listADT list, coor pos, int time);
void busGeneratePeople(listADT bus);
listADT newBuses(coor route[], int cant, paradaADT paradas, int cantParadas);
int updeteColectivos(listADT list,int  time);

#endif /*COLECTIVO_H_*/
