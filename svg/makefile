FLAGS = -O3 -wc++11-extensions

run: dm
	dm
	bbedit  *svg

dm: main.o 
	g++ main.o -o dm 

main.o: main.cpp  
	g++ -c ${FLAGS} main.cpp simple_svg_1.0.0.hpp 

clean: 
	rm *.o *.gch
