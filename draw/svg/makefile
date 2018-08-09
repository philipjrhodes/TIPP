# Makefile for SVG version of mesh drawing code.

DRAW_DIR = ${HOME}/git/TIPP/draw
ADCIRC_DIR = ${DRAW_DIR}/adcirc
FLAGS =  -O3  -std=c++11 -I${DRAW_DIR} -I${ADCIRC_DIR} -I.

run:dm nc_inundation_v6c.grd
	 	dm

dm: main.o ${ADCIRC_DIR}/adcirc.o
	g++ main.o ${ADCIRC_DIR}/adcirc.o -o dm 

main.o: main.cpp ${ADCIRC_DIR}/adcirc.h simple_svg_1.0.0.hpp ${DRAW_DIR}/Point.h
	g++ ${FLAGS} -c main.cpp 

${ADCIRC_DIR}/adcirc.o: ${ADCIRC_DIR}/adcirc.h
	${MAKE} -C ${ADCIRC_DIR}

nc_inundation_v6c.grd:
	ln -s ${ADCIRC_DIR}/nc_inundation_v6c.grd

clean: 
	rm -f dm *.o *.gch

