# ==========================================================
# TPNro 1 :  
# ==========================================================
# Materia: 
#		Sistemas Operativos
# Integrantes:		
#		Argume, Robert		
#		Azar, Daniel
#		Rodrigo, Lorenzo
# ITBA, 2009
# ==========================================================

SCRIPT PARA EJECUTAR EL PROGRAMA
================================
linuxconsole$ a

Este script pedir� que el usuario ingrese el tipo de IPC que se usar� para las
comunicaciones entre procesos. Las Opciones son:
fifo : Usa FIFOs
sock : Usa Sockets UDP
mesg : Usa Message Queue
shm  : Usa Shared Memory con sem�foros

A continuaci�n el script hace un make clean y un make seg�n el tipo de IPC 
elegido, compila, linkedita y arranca la aplicaci�n "city".


ESTADO
======

13-09-2009
==========
- Esqueleto b�sico para graficar la ciudad, reconoce sem�foros y posiciones 
  con colectivos o vac�as.
- Se intregr� todo el manejo de las l�neas, colectivos y paradas ...  pero 
  todav�a no funciona. La versi�n que se env�a mockea el manejo mencionado.
- Ya estan los 4 IPCs funcionando

