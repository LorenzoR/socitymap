//Definiciones generales


#define MAP_SERVER_NAME "MapServer"
#define LOG_SERVER_NAME "LogServer"

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

