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
#include <sys/time.h>
#include <time.h>
#include <errno.h>

//Aproximadamente 1 seg
#define TIMESTEP 800000

int alarmFlag=0;

void setTimer(void);
void setSignals(void);
void handler(int signum);
static void sigIntHandler(int sig); 
static void procesosZombies(int sig);

void mockCalculateChanges(void);

int
main(void)
{
	srand( (unsigned) time(NULL) );
	setTimer();
	setSignals();
	printf("Looping ...\n");
	int counter=0;
	while(1)
	{
		printf("Waiting counter = %d \n", counter);
		counter = (counter+1)%100;
		pause();
	}
	return 0;
}


void 
setTimer(void) 
{
	struct itimerval it;
	//Clear itimerval struct members 
	timerclear(&it.it_interval);
	timerclear(&it.it_value);
	  
	//Set timer  
	it.it_interval.tv_usec = TIMESTEP;
	it.it_value.tv_usec    = TIMESTEP;
	setitimer(ITIMER_REAL, &it, NULL);
	return;
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
    	case SIGALRM:	mockCalculateChanges();      
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

void
mockCalculateChanges(void)
{
	printf("=====================> Timer tick ... function CalculateChanges working !!!!\n");

	//alarmFlag = 1;
	return;
} 


static void 
sigIntHandler(int sig) 
{		
	signal( SIGCHLD, procesosZombies );		
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
 
