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

Este script pedirá que el usuario ingrese el tipo de IPC que se usará para las
comunicaciones entre procesos. Las Opciones son:
fifo : Usa FIFOs
sock : Usa Sockets UDP
mesg : Usa Message Queue
shm  : Usa Shared Memory con semáforos

A continuación el script hace un make clean y un make según el tipo de IPC 
elegido, compila, linkedita y arranca la aplicación "city".


ESTADO
======

13-09-2009
==========
- Esqueleto básico para graficar la ciudad, reconoce semáforos y posiciones 
  con colectivos o vacías.
- Se intregró todo el manejo de las líneas, colectivos y paradas ...  pero 
  todavía no funciona. La versión que se envía mockea el manejo mencionado.
- Ya estan los 4 IPCs funcionando

