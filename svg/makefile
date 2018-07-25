FLAGS = -O3  -std=c++11

run: dm
	dm
	bbedit  *svg

dm: main.o adcirc.o
	g++ main.o adcirc.o -o dm 

main.o: main.cpp adcirc.h simple_svg_1.0.0.hpp
	g++ ${FLAGS} -c main.cpp 

adcirc.o: adcirc.cpp adcirc.h simple_svg_1.0.0.hpp
	g++ ${FLAGS} -c adcirc.cpp

clean: 
	rm -f *.o *.gch
