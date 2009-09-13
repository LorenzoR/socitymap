//Definiciones generales


#define SERVER_NAME "trafficServer"


enum{CLIENT, SERVER};

typedef struct comm *commT;

//Funciones publicas para le manejo de comunicaciones

commT iniciarComm( int tipoComm, char *nombreServidor );
int cerrarComm( commT comm );

int establecer_conexion( commT comm );
int aceptar_conexion( commT comm );

int liberar_conexion( commT comm );

int enviar_datos( commT comm, void *datos, int dim);
int recibir_datos( commT comm, void *datos, int lim);

