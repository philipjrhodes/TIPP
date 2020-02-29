TIPP_DIR := $(shell git rev-parse --git-dir | xargs dirname)
DRAW_DIR = ${TIPP_DIR}/draw
READER_DIR = ${DRAW_DIR}/reader
GEOMETRY_DIR = ${DRAW_DIR}/geometry

# INCLUDEDIRS = -I${READER_DIR} -I${DRAW_DIR} -I${TIPP_DIR}/include
# LIBS = -lcanvas -lhpdfs -lz 
# LIBDIRS =  -L${READER_DIR} -L${DRAW_DIR} -L${TIPP_DIR}/lib

INCLUDEDIRS = -I${GEOMETRY_DIR} -I${TIPP_DIR}/include


FLAGS = -O0 -g  ${INCLUDEDIRS}
CPPFLAGS = ${FLAGS} -std=c++11


.PHONY:lib
lib: libgeometry.a

point.o: point.cpp point.h
	g++ $(CPPFLAGS) -c point.cpp

edge.o: edge.cpp edge.h
	g++ $(CPPFLAGS) -c edge.cpp

triangle.o: triangle.cpp triangle.h
	g++ $(CPPFLAGS) -c triangle.cpp

boundingBox.o: boundingBox.cpp boundingBox.h
	g++ $(CPPFLAGS) -c boundingBox.cpp


libgeometry.a: point.o edge.o triangle.o boundingBox.o
	ar rvs libgeometry.a point.o edge.o triangle.o boundingBox.o 

	

clean:
	rm -f  *.o libgeometry.a
