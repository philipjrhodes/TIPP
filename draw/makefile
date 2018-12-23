# Makefile for PDF version of mesh drawing code.



TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)
DRAW_DIR = ${TIPP_DIR}/draw
ADCIRC_DIR = ${DRAW_DIR}/adcirc
READER_DIR = ${DRAW_DIR}/reader

INCLUDEDIRS = -I${READER_DIR} -I${ADCIRC_DIR} -I${DRAW_DIR} -I${TIPP_DIR}/include
LIBS = -lreader -lhpdfs -lz 
LIBDIRS = -I${DRAW_DIR} -L${READER_DIR} -L${TIPP_DIR}/lib 

FLAGS = -g -O0 -Wno-unused-command-line-argument  ${INCLUDEDIRS}
CPPFLAGS = ${FLAGS} -std=c++11

run: dmp nc_inundation_v6c.grd
	 ./dmp
	 #open dmpdf.pdf

dmp: dmp.o libcanvas.a adcirc ${READER_DIR}/libreader.a ${TIPP_DIR}/lib/libhpdfs.a
	g++ ${CPPFLAGS} dmp.o libcanvas.a ${ADCIRC_DIR}/adcirc.o ${LIBDIRS} ${LIBS} -o dmp 

dmp.o: dmp.cpp Canvas.h PDFCanvas.h ${ADCIRC_DIR}/adcirc.h ${READER_DIR}/point.h ${READER_DIR}/triangle.h
	g++ ${CPPFLAGS} -c dmp.cpp

PDFCanvas.o: PDFCanvas.cpp PDFCanvas.h Canvas.h ${READER_DIR}/point.h ${READER_DIR}/triangle.h
	g++ ${CPPFLAGS} -c PDFCanvas.cpp
	
Canvas.o: Canvas.cpp Canvas.h ${READER_DIR}/point.h ${READER_DIR}/triangle.h
	g++ ${CPPFLAGS} -c Canvas.cpp

libcanvas.a: PDFCanvas.o Canvas.o
	ar rvs libcanvas.a PDFCanvas.o Canvas.o

.PHONY:lib
lib: libcanvas.a

.PHONY:libinstall
libinstall: libcanvas.a
	cp libcanvas.a ${TIPP_DIR}/lib
	cp PDFCanvas.h Canvas.h ${TIPP_DIR}/include

.PHONY:cleanlibinstall
cleanlibinstall:
	rm -f ${TIPP_DIR}/include/*Canvas.h ${TIPP_DIR}/lib/libcanvas.a

.PHONY:adcirc	
adcirc: ${ADCIRC_DIR}/adcirc.o ${ADCIRC_DIR}/adcirc.h
	${MAKE} -C ${ADCIRC_DIR}

${READER_DIR}/libreader.a:
	${MAKE} -C ${READER_DIR} lib
	
${TIPP_DIR}/lib/libhpdfs.a:
	${MAKE} -C ${DRAW_DIR}/pdf

nc_inundation_v6c.grd:
	ln -s ${ADCIRC_DIR}/nc_inundation_v6c.grd

.PHONY:clean
clean: 
	rm -f *.o dmp libcanvas.a


