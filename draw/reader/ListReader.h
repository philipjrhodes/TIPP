#ifndef LISTREADER_H
#define LISTREADER_H

#include <string>
#include "TIPPList.h"
#include "point.h"
#include "triangle.h"

class ListReader{

	public:
	
		ListReader(std::string vFileName, std::string tFileName){ 
		
			vertexFileName = vFileName;
			triangleFileName = tFileName;
		}

// 		std::vector<Point>  readVertices(std::ifstream &in, int numVertices);
// 		
// 		std::vector<Triangle>  readTriangles(std::ifstream &in, int numTriangles);
		
		
		point * getVertices();
		TIPPList<triangle>	getTriangles();		
		
	protected:
		void readPoints();
		void readTriangles();

	
		std::string vertexFileName;
		std::string triangleFileName;
		
		point * points = NULL;
		TIPPList<triangle>	triangles;		
};

#endif
