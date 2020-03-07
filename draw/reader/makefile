TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)
DRAW_DIR = ${TIPP_DIR}/draw
READER_DIR = ${DRAW_DIR}/reader
GEOMETRY_DIR = ${DRAW_DIR}/geometry

INCLUDEDIRS = -I${GEOMETRY_DIR} -I${READER_DIR} -I${DRAW_DIR} -I${TIPP_DIR}/include
LIBS = -lgeometry -lcanvas -lhpdfs -lz 
LIBDIRS =  -L${GEOMETRY_DIR} -L${READER_DIR} -L${DRAW_DIR} -L${TIPP_DIR}/lib

FLAGS = -O0 -g  ${INCLUDEDIRS}
CPPFLAGS = ${FLAGS} -std=c++11


default:mfd

# run: rd
# 	./rd drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
# 	open reader.pdf
# 
# rundark: rd_dark
# 	./rd_dark drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
# 	open reader.pdf
# 
# runlight: rd_light
# 	./rd_light drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
# 	open reader.pdf
# 
# .PHONY:valgrind	
# valgrind: rd
# 	valgrind  --undef-value-errors=no ./rd drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/returnAllStoreTriangleIds10.tri drawData/boundaryAndInteriorTriangles-of-fourActiveSubPartitions-of-coarsePartition10/fullPointPart.ver
# 	#--track-origins=yes

all: rd rd_dark rd_light mfd

# rd_dark: libcanvas libreader.a  maindark.o 
# 	g++ maindark.o libreader.a ${LIBDIRS}  ${LIBS} -o rd_dark
# 
# rd_light: libcanvas libreader.a mainlight.o 
# 	g++ mainlight.o libreader.a ${LIBDIRS}  ${LIBS} -o rd_light
# 	
# rd: libcanvas libreader.a main.o 
# 	g++ main.o libreader.a ${LIBDIRS}  ${LIBS} -o rd
# 	# dsymutil rd  #mac

mfd: libcanvas libgeometry libreader.a multifile.o 
	g++ multifile.o libreader.a ${LIBDIRS}  ${LIBS} -o mfd

multifile.o: multifile.cpp TIPPList.hpp 
	g++ $(CPPFLAGS) -c multifile.cpp

ListReader.o: ListReader.h ListReader.cpp TIPPList.hpp
	g++ $(CPPFLAGS) -c ListReader.cpp

# main.o: main.cpp TIPPList.hpp 
# 	g++ $(CPPFLAGS) -c main.cpp
# 
# mainlight.o: main.cpp TIPPList.hpp 
# 	g++ -DLIGHT $(CPPFLAGS) -c main.cpp -o mainlight.o
# 
# maindark.o: main.cpp TIPPList.hpp 
# 	g++ -DDARK $(CPPFLAGS) -c main.cpp -o maindark.o
	

.PHONY:lib
lib: libreader.a

libreader.a: ListReader.o   TIPPList.hpp
	ar rvs libreader.a  ListReader.o 

.PHONY:libcanvas
libcanvas:
	${MAKE} -C ${DRAW_DIR} lib
	
.PHONY:libgeometry	
libgeometry:
	${MAKE} -C ${GEOMETRY_DIR} lib
	
	

clean:
	rm -f  *.o libreader.a rd rd_dark rd_light mfd
