#ifndef PARADAS_H_
#define PARADAS_H_


typedef struct paradaT * paradaADT;
typedef struct peopleT * peopleADT;

paradaADT newparadas();
paradaADT insertParada(paradaADT list, coor pos);
int hastostop(paradaADT list, coor pos);

int haspeopletoleave(peopleADT list,  coor pos);
void removepeople(peopleADT *list,  coor pos);
void  movepeople(paradaADT list,  coor pos, peopleADT * bus);

#endif /*PARADAS_H_*/
