
FLAGS = -O0 -g
CPPFLAGS = ${FLAGS} -std=c++11

run: rd
	./rd

valgrind: rd
	valgrind --track-origins=yes rd

rd: main.o ListReader.o point.o edge.o triangle.o boundingBox.o
	g++ main.o point.o edge.o triangle.o boundingBox.o -o rd
	# dsymutil rd

main.o: main.cpp TIPPList.hpp
	g++ $(CPPFLAGS) -c main.cpp

ListReader.o: ListReader.h ListReader.cpp 
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
	rm -f  *.o rd
