#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <wchar.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "application.h"

//Global variable to store applicaton state
systemT citySystem;

//Screen ncurses auxiliary functiones
void screen_init(void);
static void update_display(void);
void screen_end(void);

//General functions to initialize and close system resources
int initializeSystem(void );
int closeSystem( void );

//Auxiliary functions to translate logical map info to screen ncurses info
void buildMap( struct mapCDT *map );
int isOneStreet(int y, int x);
void GetTermSize(int * rows, int * cols);

//Functions to send info to ncurses windows
void updateScreenLog(char *eventString);
void updateScreenMap( struct mapCDT *map );
void updateStatus( int y, int x, int status);

//To mock the changes that should be done by the main traffic process
void modifySemaphores( struct mapCDT *map);
void getRoundRoute( int y, int x , coorT **route);
void createEvents(char *events[], int dim);
void freeEvents(char *events[], int dim);

//To manage processes signals
void setSignals(void);
void handler(int signum); 

static void sigIntHandler(int sig); 
static void procesosZombies(int sig);

//Main process that calculates traffic evolution 
//and send modifications to the map 
void trafficManager(void);
void mockTrafficManager(void);


int
main(void)
{
	int pid;
	struct mapCDT *mymap;
	
	//Initialize all system resources
	initializeSystem();

	switch ( pid = fork() )
	{
		case -1:
			fatal("Error fork !!\n");
			break;

		case 0: // child		
			//trafficManager();
			mockTrafficManager();							
			exit(0);
			break;
		
		default: // father 				
			//Manage signals SIGINT, SIGCHLD, SIGTERM			
			//signal(SIGINT, siginthandler);
			//signal(SIGCHLD, procesosZombies);
			setSignals();
	}	
		
	//Simulates a log window constant update
	char *events[12];
	createEvents(events, 12);
	int i=0;
	
	while (1)
	{
		//printf("Waiting for changes ...\n");//DEBUG		
		if( getMapUpdates( &mymap ) == 0)
		{
			//printf("==> valor = %d\n", citySystem.map->matrix[2][2]);//DEBUG
			updateScreenMap( mymap );
			updateScreenLog( events[i] );			
			free(mymap);			
			update_display();			
			//emulate multiple messages for the log window
			i = (i+1) %12;
		}
   	}
	//free resources for messages emulated
	freeEvents(events, 12);
	
	closeSystem( );	
	return 0;
}

void
mockTrafficManager(void)
{
	getNewSession(CLIENT);
	struct mapCDT map2;
	coorT *route, *route2, *route3;
	
	int i=0;				
	route = malloc(sizeof(coorT)*12);
	route2 = malloc(sizeof(coorT)*12);				
	route3 = malloc(sizeof(coorT)*12);
					
	buildMap( &map2 );
	getRoundRoute(5,5,&route);
	getRoundRoute(11,11,&route2);
	getRoundRoute(14,2,&route3);
	while( 1 )
	{	
		sleep(1);					
		modifySemaphores( &map2 );
		
		//Simulate a bus in a round trip
		map2.state[route[i].y][route[i].x] = 1;					
		map2.state[route2[(i+1)%12].y][route2[(i+1)%12].x] = 1;
		map2.state[route3[(i+6)%12].y][route3[(i+6)%12].x] = 1;
		
		//Send changes on the map to the main process
		putMapUpdates( &map2 );
		
		//clean last bus position					
		map2.state[route[i].y][route[i].x] = 0;
		map2.state[route2[(i+1)%12].y][route2[(i+1)%12].x] = 0;
		map2.state[route3[(i+6)%12].y][route3[(i+6)%12].x] = 0;
		
		//To walk only through the simulation arrays
		//for RoundRobinRoutes and Log messages
		i= (i+1)%12;				
	}	
	
	free(route);
	free(route2);
	closeSession();
	return;
}

void 
trafficManager(void)
{
	/*
	lineaADT * lineas = NULL;
	int cantlinea, change, i,time=0;
	DIR *d;
	struct dirent *dir;
	*/
	//memset(&map,0xff, sizeof(struct mapCDT));
	buildMap(&map);
	newLights();
	/*
	d = opendir(DIRLINEA);
	if ( d )
	{
		readdir(d);
		readdir(d);
		while ((dir = readdir(d)) != NULL)
	    {
			cantlinea++;
			lineas =	realloc (lineas, cantlinea * sizeof(lineaADT));
			 if (lineas==NULL)
	     		fatal("Error (re)allocating memory to save the line list");
			    
	    	lineas[cantlinea -1 ] = ReadBusLine(dir->d_name);
	    }	
	    closedir(d);
	}
	*/
	
	while( 1 )
	{
		sleep(1);
		//time++;
		//change  = 1;
		//changeSemaforo(  time );
		/*
		while(change != 0)
		{
			change = 0;
			for(i= 0; i < cantlinea ; ++i)
				if (updateLinea(lineas[i], time) == 1)
					change = 1;
		}	
		*/
		putMapUpdates(&map);
	}	
	
	return;
}

int
closeSystem( void )
{
	closeSession();
	screen_end();
	return 0;
}

int
initializeSystem( void )
{
	//citySystem.map = &map;

	//initializes nCurses interface
	screen_init();
	echo();
	curs_set(0);

	//initialize session layer
	getNewSession(SERVER);

	return 0;
}

void 
getRoundRoute( int y, int x , coorT **route)
{
	int dir, i, k;	
		
	coorT aux[]={{-1,0},{0,1},{1,0},{0,-1}};	
	k=0;	
	for( dir=0 ; dir<4 ; dir++ )
	{		
		for( i=0; i<3; i++ )
		{
			(*route)[k].x =x;
			(*route)[k].y =y;
			y += aux[dir].y;
			x += aux[dir].x;			
			k++;			
		}
	}		
	return;
}

void 
createEvents(char *events[], int dim)
{
	int i, k;
	int cantStrings;
	char *strings[]={"Aparecieron personas en la parada 2, esperan la linea 60.",
					"Colectivo 3, linea 66, llego a la parada 4. Bajan personas.",
					"Colectivo 3, linea 66, esta en la parada 4. Suben personas.",
					"Colectivo 1, linea 60, llego a la parada 1. Nadie baja.",
					"Colectivo 1, linea 60, esta en la parada 1. Nadie sube.",
					"Colectivo 1, linea 60, llego a la parada 2.",
					"Colectivo 1, linea 60, esta en la parada 2. Nadie baja.",
					"Colectivo 1, linea 60, llego a la parada 2. Suben personas.",
					"Aparecieron personas en la parada 5, esperan la linea 99",
					"Colectivo 2, linea 85, llego a la parada 6. Bajan personas.",
					"Colectivo 2, linea 85, llego a la parada 6. Nadie sube.",
					"Colectivo 5, linea 92, llego a la parada 4. Nadie baja.",
					"Colectivo 5, linea 92, llego a la parada 4. Nadie sube.",
					"Colectivo 4, linea 47, sale de parada inicial"
					};
	
	cantStrings = sizeof(strings)/sizeof(strings[0]);
	
	for( i=0, k=0 ; i< dim ; i++)
	{
		events[i] = strdup(strings[k]);
		k = (k+1) % cantStrings;
	}
	return;
}

void 
freeEvents(char *events[], int dim)
{
	int i;
	if( events != NULL && dim > 0 )
	{
		for( i= 0 ; i< dim ; i++ )
			if( events[i] != NULL )
				free( events[i] );
	}
	return;
}

void
modifySemaphores( struct mapCDT *map)
{
	if( map->state[5][2] == VERDEVERTICALVACIO )
		map->state[5][2] = ROJOVERTICALVACIO;
	else
		map->state[5][2] = VERDEVERTICALVACIO;

	if( map->state[2][2] == ROJOVERTICALVACIO)
		map->state[2][2] = VERDEVERTICALVACIO;
	else
		map->state[2][2] = ROJOVERTICALVACIO;
	
	if( map->state[14][11] == ROJOVERTICALVACIO)
		map->state[14][11] = VERDEVERTICALVACIO;
	else
		map->state[14][11] = ROJOVERTICALVACIO;

	if( map->state[2][11] == ROJOVERTICALVACIO)
		map->state[2][11] = VERDEVERTICALVACIO;
	else
		map->state[2][11] = ROJOVERTICALVACIO;
	
	if( map->state[8][8] == ROJOVERTICALVACIO)
		map->state[8][8] = VERDEVERTICALVACIO;
	else
		map->state[8][8] = ROJOVERTICALVACIO;
	return;
}

void 
updateScreenLog(char *eventString)
{
	char *prompt = "=>";

	//Emulates change of color for each message line according to message type
	if( (*eventString) == 'C')
		color_set(4, NULL);
	else
		wbkgdset(citySystem.logWindow, ' ' | A_BOLD | COLOR_PAIR(6));//black,black//color_set(4, NULL);
	
	wprintw(citySystem.logWindow, "%s %s\n", prompt, eventString);
	
	//Get back the default settings
	wcolor_set(citySystem.logWindow, 1, NULL);			
	wattron(citySystem.logWindow, A_BOLD);	
	wbkgdset(citySystem.logWindow, ' ' | A_BOLD | COLOR_PAIR(1));//Verifico que el background se setee bien
	return;
}

void
updateScreenMap( struct mapCDT *map )
{
	int i,j;
	//each position in the map generates a square of 2 char by 2 chars
	for( i=0 ; i<MAXY ; i++ )
	{
		for( j=0 ; j<MAXX ; j++ )
			if( map->state[i][j] == -1 )//it is a block
			{
				wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));//black,black
				mvwaddch(citySystem.mapWindow, i*2, j*2, ' ');
				mvwaddch(citySystem.mapWindow, i*2, j*2+1, ' ');
				mvwaddch(citySystem.mapWindow, i*2+1, j*2, ' ');
				mvwaddch(citySystem.mapWindow, i*2+1, j*2+1, ' ');
			}
			else
			{
				//always print a block char in the upper left char position
				wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
				mvwaddch(citySystem.mapWindow, i*2, j*2, ' ');

				//solve what to put on the upper right char
				if( j==2 || j==5 || j==8  || j==11 || j==14 )//it is a street in vertical direction
				{
					//print a char that represents a free street position
					wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));//blue, blue
					//mvwaddch(citySystem.mapWindow, i*2, j*2+1, ' ');
				}
				else
				{
					//print a char that represents a block
					wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
					mvwaddch(citySystem.mapWindow, i*2, j*2+1, ' ');
				}

				//solve what to put on the lower left char
				if( i==2 || i==5 || i==8  || i==11 || i==14 )//it is a street in horizontal direction
				{
					//print a char that represents a free street position
					wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));//blue, blue
					//mvwaddch(citySystem.mapWindow, i*2+1, j*2, ' ');
				}
				else
				{
					//print a char that represents a block
					wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
					mvwaddch(citySystem.mapWindow, i*2+1, j*2, ' ');
				}
				//always put the map status mark on the lower right char
				updateStatus( i*2+1, j*2+1, map->state[i][j] );				
				//mvwaddch(citySystem.mapWindow, i*2+1, j*2+1, 'O');
			}
			
			//draw at the edge of the map, vertical direction
			//print a char that represents a block
			wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
			mvwaddch(citySystem.mapWindow, i*2, j*2, ' ');
			if( i==2 || i==5 || i==8  || i==11 || i==14 )//it is a street in horizontal direction
			{
				//print a char that represents a free street position
				wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));//blue, blue
				mvwaddch(citySystem.mapWindow, i*2+1, j*2, ' ');
			}
			else
			{
				//print a block
				wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
				mvwaddch(citySystem.mapWindow, i*2+1, j*2, ' ');
			}	
	}
	//draw at the edge of the map, horizontal direction	
	for( j=0 ; j<=2*MAXX ; j++ )
	{
		if( j%6 == 5 )
			wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
		else
			wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));

		mvwaddch(citySystem.mapWindow, i*2, j, ' ');
	}	
	return;
}

void
updateStatus( int y, int x, int status)
{	
	//VACIO = 0, LLENO = 1, VERDEVERTICALVACIO = 2, VERDEVERTICALLLENO = 3, 
	//ROJOVERTICALVACIO = 4, ROJOVERTICALLENO =5,  PARADAVACIO = 6, PARADASLLENO = 7
	switch( status )
	{
	case VACIO: wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));
				mvwaddch(citySystem.mapWindow, y, x, 'O');break;
	case LLENO: wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(4));
				mvwaddch(citySystem.mapWindow, y, x, ' ');break;
	case VERDEVERTICALVACIO : 	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
								mvwaddch(citySystem.mapWindow, y, x, ' ');
								wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(6));
								mvwaddch(citySystem.mapWindow, y, x+1, 'S');
								mvwaddch(citySystem.mapWindow, y, x-1, 'S');
								mvwaddch(citySystem.mapWindow, y+1, x, ' ');
								mvwaddch(citySystem.mapWindow, y-1, x, ' ');
								break;
	case ROJOVERTICALVACIO : 	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
								mvwaddch(citySystem.mapWindow, y, x, ' ');
								wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(6));
								mvwaddch(citySystem.mapWindow, y+1, x, 'S');
								mvwaddch(citySystem.mapWindow, y-1, x, 'S');
								mvwaddch(citySystem.mapWindow, y, x+1, ' ');
								mvwaddch(citySystem.mapWindow, y, x-1, ' ');
								break;
	case VERDEVERTICALLLENO : 	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
								mvwaddch(citySystem.mapWindow, y, x, ' ');
								wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(4));
								mvwaddch(citySystem.mapWindow, y, x+1, 'S');
								mvwaddch(citySystem.mapWindow, y, x-1, 'S');
								mvwaddch(citySystem.mapWindow, y+1, x, ' ');
								mvwaddch(citySystem.mapWindow, y-1, x, ' ');
								break;
	case ROJOVERTICALLENO : 	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
								mvwaddch(citySystem.mapWindow, y, x, ' ');
								wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(4));
								mvwaddch(citySystem.mapWindow, y+1, x, 'S');
								mvwaddch(citySystem.mapWindow, y-1, x, 'S');
								mvwaddch(citySystem.mapWindow, y, x+1, ' ');
								mvwaddch(citySystem.mapWindow, y, x-1, ' ');
								break;
	case PARADAVACIO:	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));
						mvwaddch(citySystem.mapWindow, y, x, 'O');
						wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(13));
						if( x == 5 || x == 11 || x == 17 || x == 23 || x == 29 )
							mvwaddch(citySystem.mapWindow, y, x-1, 'P');
						if( y == 5 || y == 11 || y == 17 || y == 23 || y == 29 )
							mvwaddch(citySystem.mapWindow, y-1, x, 'P');
						break;
	case PARADASLLENO:	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(4));
						mvwaddch(citySystem.mapWindow, y, x, ' ');						
						wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(13));
						if( x == 5 || x == 11 || x == 17 || x == 23 || x == 29 )
							mvwaddch(citySystem.mapWindow, y, x-1, 'P');
						if( y == 5 || y == 11 || y == 17 || y == 23 || y == 29 )
							mvwaddch(citySystem.mapWindow, y-1, x, 'P');
						break;
	default: 	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
				mvwaddch(citySystem.mapWindow, y, x, ' ');
	}
	return;
}

void
buildMap( struct mapCDT *map)
{
	int i,j;
	
	memset(map->state,0xff, sizeof(int)*MAXX*MAXY);
		
	for( i=0 ; i<MAXY ; i++ )
		for( j=0 ; j<MAXX ; j++ )
			if( isOneStreet(i,j) )
				map->state[i][j] = VACIO;
	/*To emulate initial states for semaphores
	map->state[1][2] = LLENO;
	map->state[5][2] = VERDEVERTICALVACIO;
	map->state[2][2] = ROJOVERTICALVACIO;
	*/
	map->state[7][8] = PARADASLLENO;	
	map->state[14][15] = PARADASLLENO;
	map->state[5][13] = PARADASLLENO;
	map->state[1][2] = PARADASLLENO;
	
	map->state[14][0] = PARADAVACIO;
	map->state[8][1] = PARADAVACIO;
	map->state[11][1] = PARADAVACIO;
	map->state[2][15] = PARADAVACIO;
	return;
}

int
isOneStreet(int y, int x)
{
	if( x == 2 || x == 5 || x == 8 || x == 11 || x == 14 )
		return TRUE;
	if( y == 2 || y == 5 || y == 8 || y == 11 || y == 14 )
		return TRUE;

	return FALSE;
}

void 
GetTermSize(int * rows, int * cols) 
{
    struct winsize ws;
    //Get terminal size  
    if ( ioctl(0, TIOCGWINSZ, &ws) < 0 ) 
        fatal("Couldn't get window size");    
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return;
}

void
screen_init(void)
{
	int rows, cols;
	
	if( (citySystem.mainWindow = initscr()) == NULL )
		fatal("ERROR while initializing ncurses\n");
	keypad( stdscr, TRUE );
	nonl();
	noecho();
	cbreak();
   	intrflush(stdscr, FALSE);
   	curs_set(1);
	//Color pairs to use
	if( has_colors() )
	{
		start_color();
		init_pair(1,  COLOR_YELLOW,  COLOR_BLUE);
		init_pair(2,  COLOR_BLUE,    COLOR_BLUE);
		init_pair(3,  COLOR_BLACK,   COLOR_GREEN);
		init_pair(4,  COLOR_WHITE,   COLOR_RED);
		init_pair(5,  COLOR_WHITE,   COLOR_GREEN);
		init_pair(6,  COLOR_RED,     COLOR_BLUE);
		init_pair(7,  COLOR_BLACK,	 COLOR_BLACK);
		init_pair(8,  COLOR_MAGENTA, COLOR_BLACK);
		init_pair(9,  COLOR_GREEN,	 COLOR_BLACK);
	   	init_pair(10, COLOR_BLUE,    COLOR_YELLOW);
	   	init_pair(11, COLOR_WHITE,   COLOR_YELLOW);
	   	init_pair(12, COLOR_WHITE,   COLOR_MAGENTA);
	   	init_pair(13, COLOR_CYAN,   COLOR_BLACK);
	}
	
   	nodelay(citySystem.mainWindow, TRUE);
   	
   	//Check if term size is not enough for the map
   	GetTermSize(&rows,&cols);
   	if( rows < MIN_ROWS || cols < MIN_COLS )
   		resizeterm(MIN_ROWS, MIN_COLS);
   	
   	//GENERATE MAP WINDOW AND LOG WINDOW
   	//Frame for Map Window
   	citySystem.mapWindowFrame = newwin(MAP_WINDOW_HEIGHT, MAP_WINDOW_WIDTH, ORIGIN_Y, ORIGIN_X);   	
   	wcolor_set(citySystem.mapWindowFrame, 1, NULL); //Color pair to be used in the city map
   	wattron(citySystem.mapWindowFrame, A_BOLD);	//Make Colors stronger
   	wbkgdset(citySystem.mapWindowFrame, ' ' | A_BOLD | COLOR_PAIR(1));//Check for background to be set correctly
   	wclear(citySystem.mapWindowFrame);//Start with a clean window
   	box(citySystem.mapWindowFrame, ACS_VLINE, ACS_HLINE);//Draw a box
   	mvwaddstr(citySystem.mapWindowFrame, 0,2, " MAPA ");
   	
   	//Window for the map itself
   	citySystem.mapWindow = derwin(citySystem.mapWindowFrame, MAP_WINDOW_HEIGHT-2, MAP_WINDOW_WIDTH-3, 2, 3);//Ventana derivada de la anterior
   	wcolor_set(citySystem.mapWindow, 1, NULL);			
   	wattron(citySystem.mapWindow, A_BOLD);	
   	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(1));//Verifico que el background se setee bien
   	wclear(citySystem.mapWindow);//Start with a clean window
   	scrollok(citySystem.mapWindow, TRUE);
   	
   	//Frame for Log Window
   	//citySystem.logWindowFrame = newwin(LOG_WINDOW_HEIGHT, LOG_WINDOW_WIDTH, ORIGIN_Y, ORIGIN_X+MAP_WINDOW_WIDTH);
   	citySystem.logWindowFrame = newwin(0, 0, ORIGIN_Y, ORIGIN_X+MAP_WINDOW_WIDTH);
   	wcolor_set(citySystem.logWindowFrame, 1, NULL); //Color pair to be used in the city map
   	wattron(citySystem.logWindowFrame, A_BOLD);	//Make Colors stronger
   	wbkgdset(citySystem.logWindowFrame, ' ' | A_BOLD | COLOR_PAIR(1));//Check for background to be set correctly
   	wclear(citySystem.logWindowFrame);//Start with a clean window
   	box(citySystem.logWindowFrame, ACS_VLINE, ACS_HLINE);//Draw a box
   	mvwaddstr(citySystem.logWindowFrame, 0,2, " LOG ");
   	
   	//Window for the log console
   	citySystem.logWindow = derwin(citySystem.logWindowFrame, 0, 0, 2, 3);//Ventana derivada de la anterior
   	wcolor_set(citySystem.logWindow, 1, NULL);			
   	wattron(citySystem.logWindow, A_BOLD);	
   	wbkgdset(citySystem.logWindow, ' ' | A_BOLD | COLOR_PAIR(1));//Verifico que el background se setee bien
   	wclear(citySystem.logWindow);//Start with a clean window
   	scrollok(citySystem.logWindow, TRUE); 	
   	   	
   	refresh();
   	return;
}

static void
update_display(void){	
  	box(citySystem.mapWindowFrame, ACS_VLINE, ACS_HLINE);
  	mvwaddstr(citySystem.mapWindowFrame, 0,2, " MAPA ");	   	
  	wrefresh(citySystem.mapWindowFrame);
  	wrefresh(citySystem.mapWindow);
  	box(citySystem.logWindowFrame, ACS_VLINE, ACS_HLINE);
  	mvwaddstr(citySystem.logWindowFrame, 0,2, " LOG ");	   	
  	wrefresh(citySystem.logWindowFrame);
  	wrefresh(citySystem.logWindow);  	
  	
  	return;
}

void screen_end(void) {
	delwin(citySystem.mapWindowFrame);
	delwin(citySystem.mapWindow);
	delwin(citySystem.logWindowFrame);
	delwin(citySystem.logWindow);
	endwin();
}

//Sets up signal handlers we need 
void 
setSignals(void) 
{
    struct sigaction sa;
    
    //Fill in sigaction struct 
    sa.sa_handler = handler;
    sa.sa_flags   = 0;
    sigemptyset(&sa.sa_mask);
    
    //Set signal handlers 
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);
    
    //Ignore SIGTSTP  
    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);
}

//Signal generic handler 
void 
handler(int signum) 
{   
    switch ( signum ) 
    {
    	case SIGALRM:	//In case we use timers      
		      			return;
		      			
    	case SIGTERM:    		
    	case SIGINT:	sigIntHandler(SIGINT);
    					break;
    					
    	case SIGCHLD: 	procesosZombies(SIGCHLD);
    					break;
    					
    	default:		break;
    }
    return;
}

static void 
sigIntHandler(int sig) 
{		
	signal( SIGCHLD, procesosZombies );	
	closeSystem();
	printf("\tCityMap is down... bye !!\n");
	exit(EXIT_SUCCESS);
} 

static void
procesosZombies(int sig)
{
	int status;
	printf("Process %d killing zombies !!\n", getpid());
	while( wait3(&status, WNOHANG, (struct rusage *)0) > 0 )
		;
	return;
}
