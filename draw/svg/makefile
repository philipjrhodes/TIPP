# Makefile for SVG version of mesh drawing code.

ADCIRC_DIR = ${HOME}/git/TIPP/draw/adcirc
FLAGS = -v -O3  -std=c++11 -I${ADCIRC_DIR} -I.

run:dm nc_inundation_v6c.grd
	 	dm

dm: main.o ${ADCIRC_DIR}/adcirc.o
	g++ main.o ${ADCIRC_DIR}/adcirc.o -o dm 

main.o: main.cpp ${ADCIRC_DIR}/adcirc.o simple_svg_1.0.0.hpp
	g++ ${FLAGS} -c main.cpp 

${ADCIRC_DIR}/adcirc.o:
	${MAKE} -C ${ADCIRC_DIR}

nc_inundation_v6c.grd:
	ln -s ${ADCIRC_DIR}/nc_inundation_v6c.grd

clean: 
	rm -f *.o *.gch

