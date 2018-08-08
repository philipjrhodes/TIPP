# Makefile for adcirc file reader code.

DRAW_DIR = ${HOME}/git/TIPP/draw
FLAGS = -O3  -std=c++11 -I${DRAW_DIR}

adcirc.o: adcirc.cpp adcirc.h ${DRAW_DIR}/Point.h
	g++ ${FLAGS} -c adcirc.cpp

clean: 
	rm -f  *.o 
