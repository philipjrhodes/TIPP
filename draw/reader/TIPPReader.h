#ifndef TIPPREADER_H
#define TIPPREADER_H

#include <stdio.h>
#include <string>
#include "TIPPkList.h"


class TIPPReader {
	public:
	
		TIPPReader(string filename);
	
		//TODO: change vector to TIPP list
		std::vector<Point>  readVertices(FILE * in, int numVertices);

		std::vector<Triangle>  readTriangles(FILE * in, int numTriangles);

		TIPPList<Point>  readVertices(FILE * in, int numVertices);

		TIPPList<Triangle> readTriangles(FILE * in, int numTriangles);
}

#endif


