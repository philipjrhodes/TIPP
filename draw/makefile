# Makefile for PDF version of mesh drawing code.

TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)
DRAW_DIR = ${TIPP_DIR}/draw
ADCIRC_DIR = ${DRAW_DIR}/adcirc

INCLUDEDIRS = -I${ADCIRC_DIR} -I${DRAW_DIR} -I${TIPP_DIR}/include
LIBS = -lhpdfs -lz
LIBDIRS = -L${TIPP_DIR}/lib 

FLAGS = -O3 -Wno-unused-command-line-argument ${LIBDIRS} ${INCLUDEDIRS}
CPPFLAGS = ${FLAGS} -std=c++11

run: dmp nc_inundation_v6c.grd
	 ./dmp
	 #open dmpdf.pdf

dmp: dmp.o PDFCanvas.o Canvas.o ${ADCIRC_DIR}/adcirc.o
	g++ ${CPPFLAGS} dmp.o PDFCanvas.o Canvas.o ${ADCIRC_DIR}/adcirc.o ${LIBS} -o dmp 

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


