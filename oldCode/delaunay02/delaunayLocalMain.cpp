//g++ -std=gnu++11 linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp delaunayLocal.cpp delaunayLocalMain.cpp -o delaunayLocalMain

#include <iostream>
#include "delaunayLocal.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	if(argc==1){// no arguments
		std::cout<<"You need to provide an arguments: path.\n";
		std::cout<<"For example, ../dataSources/1Kvertices/\n";
	}
	else{
		std::string path = argv[1];
		delaunayLocal *dl = new delaunayLocal(path);
		dl->readTriangleData();
		dl->partTriangulate();
		delete dl;
	}
}


