# Makefile for PDF version of mesh drawing code.


#SHELL = /bin/csh

ADCIRC_DIR = ${HOME}/git/TIPP/draw/adcirc
FLAGS = -L${HOME}/git/TIPP/lib -I${ADCIRC_DIR} -I. -lhpdf

run: dmpdf
	dmpdf
	open dmpdf.pdf

dmpdf: dmpdf.o
	gcc ${FLAGS} dmpdf.o -o dmpdf 

dmpdf.o: dmpdf.c
	gcc ${FLAGS} -c dmpdf.c 

${ADCIRC_DIR}/adcirc.o:
	${MAKE} -C ${ADCIRC_DIR}

clean: 
	rm -f *.o dmpdf


