# Makefile for adcirc file reader code.

SVG_DIR = ${HOME}/git/TIPP/draw/svg
FLAGS = -O3  -std=c++11 -I${SVG_DIR}

adcirc.o: adcirc.cpp adcirc.h ${SVG_DIR}/simple_svg_1.0.0.hpp
	g++ ${FLAGS} -c adcirc.cpp

clean: 
	rm -f  *.o 
