#ifndef LINEA_H_
#define LINEA_H_
#define DIRLINEA "./linea/"
typedef struct lineaT * lineaADT;

//lineaADT newlinea(coor route[], int cant, paradaADT paradas, int busesToSend, coor start ,int timeToStart);
int updateLinea (lineaADT line, int time);
lineaADT ReadBusLine(char * arch);
void generatePeople(lineaADT linea );
#endif /*LINEA_H_*/
