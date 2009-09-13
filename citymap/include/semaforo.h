#ifndef SEMAFORO_H_
#define SEMAFORO_H_

#define ARCHSEMAFORO "semaforos"
typedef struct semaforoT * semaforoADT;

void changeSemaforo( int time );
void newLights(void);
#endif /*SEMAFORO_H_*/
