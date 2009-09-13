#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <dirent.h>

//check
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <errno.h>


//our includes
#include "mapADT.h"
#include "application.h"
#include "session.h"
#include "common.h"
#include "semaforo.h"
#include "linea.h"


//---------------------------------------------------------------------------
//DEFINITIONS FOR THE SCREEN MAP MADE IN NCURSES
//	static system->mainWindow WINDOW *mainWindow;
//	static WINDOW *mapWindow;

//Origen de coordenadas para la ventana del mapa
#define ORIGIN_X 	0
#define ORIGIN_Y 	0

//other posibility is 30x92
#define	MAX_LIN		40//25
#define	MAX_COL 	100//80

//number of blocks in X an in Y
#define BLOCKS_Y 6
#define BLOCKS_X 6

//dimensions for the blocks
#define BLOCK_HEIGHT 4
#define BLOCK_WIDTH 5

//separation among blocks
#define STREET_SIZE_Y 1
#define STREET_SIZE_X 1

//screen dimensions
#define SCREEN_SIZE_Y   BLOCK_HEIGHT*BLOCKS_Y+STREET_SIZE_Y*(BLOCKS_Y-1)
#define SCREEN_SIZE_X   BLOCK_WIDTH*BLOCKS_X+STREET_SIZE_X*(BLOCKS_X-1)

#define SPEED_RATE 1

//-------------------------------------------------------------------------

//State for the application
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
void updateScreenMap( struct mapCDT *map, WINDOW *mapWindow);
void updateStatus( WINDOW *mapWindow, int y, int x, int status);

//To mock the process that put changes on the map
void modifySemaphores( struct mapCDT *map);
void getRoundRoute( int y, int x , coorT **route);



//To manage processes abrupt end
static void siginthandler(int sig); 
static void procesosZombies(int sig);

//Main process that make updates 
void core(void);

int
main(void)
{
	int pid;
	
	//Initialize all system resources
	initializeSystem();
				
		switch ( pid = fork() )
		{
			case -1:
				fatal("Error fork !!\n");
				break;
	
			case 0: // child
				
				getNewSession(CLIENT);
				//core();
				
				struct mapCDT map2;
				coorT *route;
				int i=0;				
				route = malloc(sizeof(coorT)*12);
								
				buildMap( &map2 );
				getRoundRoute(5,5,&route);
				
				while( 1 )
				{	
					sleep(1);
					modifySemaphores( &map2 );
					
					//Simulate a bus in a round trip
					map2.state[route[i].y][route[i].x] = 1;					
				
					//Send changes on the map to the main process
					putMapUpdates( &map2 );
					
					//clean last bus position					
					map2.state[route[i].y][route[i].x] = 0;
					i= (i+1)%12;										
				}					
				
				closeSession();
								
				exit(0);
				break;
			
			default: // father 				
				//Capturo signals CTRL+C, SIGPIP y SIGCHLD			
				signal(SIGINT, siginthandler);
				signal(SIGCHLD, procesosZombies);
		}
	
	getNewSession(SERVER);
	struct mapCDT *mymap;
	
	/*
	while(1)
	{
		buildMap( citySystem.map );
		//wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));//Verifico que el background se setee bien
		
		updateScreenMap(citySystem.map, citySystem.mapWindow);
		wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));//Verifico que el background se setee bien
		mvwaddch(citySystem.mapWindow, 0, 0, 'W');
		mvwaddch(citySystem.mapWindow, 10, 10, 'Q');
		wrefresh(citySystem.mapWindow);
		sleep(1);
	}
	*/
	
	while (1)
	{
		//printf("Waiting for changes ...\n");
		//modifySemaphores( &(citySystem.map) );
		//mvwprintw(system.mapWindow,40,40,"BEFORE GETMAPUPDATES\n");
		//wrefresh(citySystem.mapWindow);
		
		if( getMapUpdates( &mymap ) == 0)
		{
			updateScreenMap(mymap, citySystem.mapWindow);
			free(mymap);
			//printf("==> valor = %d\n", citySystem.map->matrix[2][2]);
			//wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));//Verifico que el background se setee bien
			//mvwaddch(citySystem.mapWindow, 0, 0, 'W');
			wrefresh(citySystem.mapWindow);
			
			//sleep(1);
		}
   	}
	
	closeSystem( );
	printf("TEST ENDS\n");
	return 0;
}

void 
core(void)
{
	lineaADT * lineas = NULL;
	int cantlinea, change, i,time=0;
	DIR *d;
	struct dirent *dir;
	
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
		time++;
		change  = 1;
		changeSemaforo(  time );
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
	//build logical map ... just a mocked map	
	//buildMap( &(citySystem.map) );
	
	citySystem.map = &map;

	//initializes nCurses interface
	screen_init();
	echo();
	curs_set(0);

	//initialize session layer
	//getNewSession(SERVER);

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

	return;
}

void
updateScreenMap( struct mapCDT *map, WINDOW *mapWindow)
{
	int i,j;
	//each position in the map generates a square of 2 char by 2 chars
	for( i=0 ; i<MAXY ; i++ )
	{
		for( j=0 ; j<MAXX ; j++ )
			if( map->state[i][j] == -1 )//it is a block
			{
				wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));//black,black
				mvwaddch(mapWindow, i*2, j*2, ' ');
				mvwaddch(mapWindow, i*2, j*2+1, ' ');
				mvwaddch(mapWindow, i*2+1, j*2, ' ');
				mvwaddch(mapWindow, i*2+1, j*2+1, ' ');
			}
			else
			{
				//always print a block char in the upper left char position
				wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
				mvwaddch(mapWindow, i*2, j*2, ' ');

				//solve what to put on the upper right char
				if( j==2 || j==5 || j==8  || j==11 || j==14 )//it is a street in vertical direction
				{
					//print a char that represents a free street position
					wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));//blue, blue
					//mvwaddch(mapWindow, i*2, j*2+1, ' ');
				}
				else
				{
					//print a char that represents a block
					wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
					mvwaddch(mapWindow, i*2, j*2+1, ' ');
				}

				//solve what to put on the lower left char
				if( i==2 || i==5 || i==8  || i==11 || i==14 )//it is a street in horizontal direction
				{
					////print a char that represents a free street position
					wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));//blue, blue
					//mvwaddch(mapWindow, i*2+1, j*2, ' ');
				}
				else
				{
					//print a char that represents a block
					wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
					mvwaddch(mapWindow, i*2+1, j*2, ' ');
				}
				//always put the map status mark on the lower right char
				updateStatus( mapWindow, i*2+1, j*2+1, map->state[i][j]);
				//wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));
				//mvwaddch(mapWindow, i*2+1, j*2+1, 'O');
			}
			
			//draw at the edge of the map, vertical direction
			////print a char that represents a block
			wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
			mvwaddch(mapWindow, i*2, j*2, ' ');
			if( i==2 || i==5 || i==8  || i==11 || i==14 )//it is a street in horizontal direction
			{
				////print a char that represents a free street position
				wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));//blue, blue
				mvwaddch(mapWindow, i*2+1, j*2, ' ');
			}
			else
			{
				//print a block
				wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));
				mvwaddch(mapWindow, i*2+1, j*2, ' ');
			}
			
	}
	//draw at the edge of the map, horizontal direction
	
	for( j=0 ; j<=2*MAXX ; j++ )
	{
		if( j%6 == 5 )
			wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
		else
			wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(7));

		mvwaddch(mapWindow, i*2, j, ' ');
	}
	
	
	return;
}

void
updateStatus( WINDOW *mapWindow, int y, int x, int status)
{
	switch( status )
	{
	case VACIO: wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(3));
				mvwaddch(mapWindow, y, x, 'O');break;
	case LLENO: wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(4));
				mvwaddch(mapWindow, y, x, ' ');break;
	case VERDEVERTICALVACIO : 	wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
								mvwaddch(mapWindow, y, x, ' ');
								wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(6));//red, black
								mvwaddch(mapWindow, y, x+1, 'S');
								mvwaddch(mapWindow, y, x-1, 'S');
								mvwaddch(mapWindow, y+1, x, ' ');
								mvwaddch(mapWindow, y-1, x, ' ');
								break;
	case ROJOVERTICALVACIO : 	wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
								mvwaddch(mapWindow, y, x, ' ');
								wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(6));//red, black
								mvwaddch(mapWindow, y+1, x, 'S');
								mvwaddch(mapWindow, y-1, x, 'S');
								mvwaddch(mapWindow, y, x+1, ' ');
								mvwaddch(mapWindow, y, x-1, ' ');
								break;
								
	default: 	wbkgdset(mapWindow, ' ' | A_BOLD | COLOR_PAIR(2));
				mvwaddch(mapWindow, y, x, ' ');
	}
	return;
}

void
buildMap( struct mapCDT *map)
{
	int i,j;
	//map->dim = MAP_SIZE;
	memset(map->state,0xFF, sizeof(int)*MAXX*MAXY);
		
	for( i=0 ; i<MAXY ; i++ )
		for( j=0 ; j<MAXX ; j++ )
			if( isOneStreet(i,j) )
				map->state[i][j] = VACIO;
	/*
	map->state[1][2] = LLENO;
	map->state[5][2] = VERDEVERTICALVACIO;
	map->state[2][2] = ROJOVERTICALVACIO;
	*/
	return;
}

int
isOneStreet(int y, int x){
	if( x == 2 || x == 5 || x == 8 || x == 11 || x == 14 )
		return TRUE;
	if( y == 2 || y == 5 || y == 8 || y == 11 || y == 14 )
		return TRUE;

	return FALSE;
}

void
screen_init(void)
{
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
	         init_pair(1, COLOR_YELLOW, COLOR_BLUE);
	         init_pair(2, COLOR_BLUE, COLOR_BLUE);
	         init_pair(3, COLOR_BLACK, COLOR_GREEN);
	         init_pair(4, COLOR_WHITE, COLOR_RED);
	         init_pair(5, COLOR_WHITE, COLOR_GREEN);
		 init_pair(6, COLOR_RED, COLOR_BLUE);
		 init_pair(7, COLOR_BLACK, COLOR_BLACK);
	}
   	nodelay(citySystem.mainWindow, TRUE);


   	//Generate the map window

   	citySystem.mapWindow = newwin(MAX_LIN, MAX_COL, ORIGIN_X, ORIGIN_Y);
   	wcolor_set(citySystem.mapWindow, 1, NULL); //Color pair to be used in the city map
   	wattron(citySystem.mapWindow, A_BOLD);	//Make Colors stronger
   	//box(mapWindow, ACS_VLINE, ACS_HLINE);//Draw a box
	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(1));//Check for background to be set correctly
   	wclear(citySystem.mapWindow);//Start with a clean window

   	refresh();

   	scrollok(citySystem.mapWindow, TRUE);
   	refresh();
   	return;
}

static void
update_display(void){
	wbkgdset(citySystem.mapWindow, ' ' | A_BOLD | COLOR_PAIR(1));//Verifico que el background se setee bien
	wclear(citySystem.mapWindow);
   	wrefresh(citySystem.mapWindow);
	//sleep(SPEED_RATE);
  	refresh();
}

void screen_end(void) {
	delwin(citySystem.mapWindow);
	endwin();
}


static void 
siginthandler(int sig) 
{		
	signal( SIGCHLD, procesosZombies );	
	closeSystem();
	printf("\tCityMap is down... bye !!\n");
	exit(1);
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
