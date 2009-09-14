# ==========================================================
# Make file 
# ==========================================================
# Materia: 
#		Ssistemas Operativos
# Integrantes:		
#		Argume, Robert		
#		Azar, Daniel
#		Rodrigo, Lorenzo
# ITBA, 2009
# ==========================================================


CC=gcc
TARGET_OPTS=-Wall -g  -o 
OBJ_OPTS=-Wall -g  -c -o 
INCLUDE_DIR=include/
SRC_DIR=src/

TARGET=city
 
COMMON=$(SRC_DIR)application.o $(SRC_DIR)session.o $(SRC_DIR)helper.o  $(SRC_DIR)semaforo.o $(SRC_DIR)mapADT.o $(SRC_DIR)linea.o $(SRC_DIR)colectivo.o $(SRC_DIR)paradas.o 
SHM=$(SRC_DIR)commShm.o $(SRC_DIR)semaphore.o $(SRC_DIR)sharedMem.o
FIFO=$(SRC_DIR)commFifo.o
SOCK=$(SRC_DIR)commSockets.o
MESG=$(SRC_DIR)commMsq.o

OBJS_SHM=$(SHM) $(COMMON)
OBJS_FIFO=$(FIFO) $(COMMON)
OBJS_SOCK=$(SOCK) $(COMMON)
OBJS_MESG=$(MESG) $(COMMON)

INCLUDE	= -I $(INCLUDE_DIR) 

all:$(OBJS_FIFO)
	$(CC) $(TARGET_OPTS) $(TARGET) $(OBJS_FIFO) -lncurses

fifo:$(OBJS_FIFO)
	 $(CC) $(TARGET_OPTS) $(TARGET) $(OBJS_FIFO) -lncurses

shm:$(OBJS_SHM)
	 $(CC) $(TARGET_OPTS) $(TARGET) $(OBJS_SHM) -lncurses
	 
sock:$(OBJS_SOCK)
	 $(CC) $(TARGET_OPTS) $(TARGET) $(OBJS_SOCK) -lncurses

mesg:$(OBJS_MESG)
	 $(CC) $(TARGET_OPTS) $(TARGET) $(OBJS_MESG) -lncurses
	 
clean:
	@-rm -v -f $(OBJS_SHM)
	@-rm -v -f $(OBJS_MESG)
	@-rm -v -f $(OBJS_SOCK)
	@-rm -v -f $(OBJS_FIFO)
	@-rm -v -f $(COMMON)
	@-rm -v -f $(TARGET)	
	
.c.o:
	$(CC) $(OBJ_OPTS) $@ $(INCLUDE) $<  
