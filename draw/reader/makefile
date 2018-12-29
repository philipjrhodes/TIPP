TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)
DRAW_DIR = ${TIPP_DIR}/draw
READER_DIR = ${DRAW_DIR}/reader


INCLUDEDIRS = -I${READER_DIR} -I${DRAW_DIR} -I${TIPP_DIR}/include
LIBS = -lcanvas -lhpdfs -lz 
LIBDIRS =  -L${READER_DIR} -L${DRAW_DIR} -L${TIPP_DIR}/lib

FLAGS = -O0 -g  ${INCLUDEDIRS}
CPPFLAGS = ${FLAGS} -std=c++11


run: rd
	#./rd drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
	./rd drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
	open reader.pdf

rundark: rd_dark
	./rd_dark drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
	open reader.pdf

runlight: rd_light
	./rd_light drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
	open reader.pdf
	
valgrind: rd
	valgrind --track-origins=yes rd

all: rd rd_dark rd_light

rd_dark: maindark.o libreader.a libcanvas
	g++ maindark.o libreader.a ${LIBDIRS}  ${LIBS} -o rd_dark

rd_light: mainlight.o libreader.a libcanvas
	g++ mainlight.o libreader.a ${LIBDIRS}  ${LIBS} -o rd_light
	
rd: main.o libreader.a libcanvas
	g++ main.o libreader.a ${LIBDIRS}  ${LIBS} -o rd
	# dsymutil rd  #mac

main.o: main.cpp TIPPList.hpp 
	g++ $(CPPFLAGS) -c main.cpp

mainlight.o: main.cpp TIPPList.hpp 
	g++ -DLIGHT $(CPPFLAGS) -c main.cpp -o mainlight.o

maindark.o: main.cpp TIPPList.hpp 
	g++ -DDARK $(CPPFLAGS) -c main.cpp -o maindark.o
	

.PHONY:lib
lib: libreader.a

libreader.a: ListReader.o point.o edge.o triangle.o boundingBox.o TIPPList.hpp
	ar rvs libreader.a  ListReader.o point.o edge.o triangle.o boundingBox.o

.PHONY:libcanvas
libcanvas:
	${MAKE} -C ${DRAW_DIR} lib
	
ListReader.o: ListReader.h ListReader.cpp TIPPList.hpp
	g++ $(CPPFLAGS) -c ListReader.cpp
	
point.o: point.cpp point.h
	g++ $(CPPFLAGS) -c point.cpp

edge.o: edge.cpp edge.h
	g++ $(CPPFLAGS) -c edge.cpp

triangle.o: triangle.cpp triangle.h
	g++ $(CPPFLAGS) -c triangle.cpp

boundingBox.o: boundingBox.cpp boundingBox.h
	g++ $(CPPFLAGS) -c boundingBox.cpp

clean:
	rm -f  *.o libreader.a rd rd_dark rd_light
