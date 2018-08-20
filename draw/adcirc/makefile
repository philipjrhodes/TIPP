# Makefile for adcirc file reader code.

TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)
DRAW_DIR = $(TIPP_DIR)/draw
ADCIRC_DIR = ${DRAW_DIR}/adcirc
READER_DIR = $(TIPP_DIR)/draw/reader

INCLUDEDIRS = -I${ADCIRC_DIR} -I${READER_DIR}
FLAGS = -O0  ${INCLUDEDIRS}
CPPFLAGS = ${FLAGS} -std=c++11

adcirc.o: adcirc.cpp adcirc.h ${READER_DIR}/point.h ${READER_DIR}/triangle.h
	g++ ${FLAGS} -c adcirc.cpp

clean: 
	rm -f  *.o 


