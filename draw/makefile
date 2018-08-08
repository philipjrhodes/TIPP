# Makefile for PDF version of mesh drawing code.

#SHELL = /bin/csh

#DRAW_DIR = ${HOME}/git/TIPP/draw
DRAW_DIR = ${PWD}
ADCIRC_DIR = ${DRAW_DIR}/adcirc

INCLUDEDIRS = -I${ADCIRC_DIR} -I.
LIBS = -lhpdfs -lz
LIBDIRS = -L${HOME}/git/TIPP/lib 

FLAGS = -O3 -Wno-unused-command-line-argument ${LIBDIRS} ${INCLUDEDIRS} ${LIBS}
CPPFLAGS = ${FLAGS} -std=c++11

run: dmp nc_inundation_v6c.grd
	 dmp
	 open dmpdf.pdf

dmp: dmp.o PDFCanvas.o Canvas.o ${ADCIRC_DIR}/adcirc.o
	g++ ${FLAGS} dmp.o PDFCanvas.o Canvas.o ${ADCIRC_DIR}/adcirc.o -o dmp 

dmp.o: dmp.cpp Canvas.h PDFCanvas.h ${ADCIRC_DIR}/adcirc.h Point.h Triangle.h
	g++ ${CPPFLAGS} -c dmp.cpp

PDFCanvas.o: PDFCanvas.cpp PDFCanvas.h Canvas.h Point.h Triangle.h
	g++ ${CPPFLAGS} -c PDFCanvas.cpp
	
Canvas.o: Canvas.cpp Canvas.h Point.h Triangle.h
	g++ ${CPPFLAGS} -c Canvas.cpp
	 
${ADCIRC_DIR}/adcirc.o:
	${MAKE} -C ${ADCIRC_DIR}

nc_inundation_v6c.grd:
	ln -s ${ADCIRC_DIR}/nc_inundation_v6c.grd

clean: 
	rm -f *.o dmp


