typedef struct{
//Position in the map
  int x;
  int y
}coor;

typedef struct{
	//Position in the map
	coor pos;	
	//segment destination point
	coor NextRoute;
	int timestop;//the time to wait to move if it is on a bus stop.
	int lastMovedOn;
}colectivoT;



typedef struct{
	//Position in the map
	coor pos;
	//Direction facing
	int state; //NS or EW
	int timechange;//the time to wait tochange the state.
	
}semaforoT;


typedef struct{
	//route in the map
	coor * route;
	int cantRoute;
	//position of the start of the line
	coor start; 
	int timeToStart;//the time to to send another bus.
	int busesToSend;
	int BusesSended;
	colectivoT * micros;
	paradaT * paradas;
}lineaT;



typedef struct{
	//Position in the map
	coor pos;
	//if people > 0 , the bus have to stop
	int people;
	
}paradaT;

/*
para la comunicacion entre objetos yo diria que hay que enviar siempre 1 int 
con el tipo , ej coelctivo = 1 , semaforo = 2, etc. , otro int con la cantidad 
ej  1 10 indicaria 10 colectivos y despues mandar un array con los elementos. 
y mandar 1 int solo o una signal caudno halla pasado un seg asi mover todos 
los objetos
*/