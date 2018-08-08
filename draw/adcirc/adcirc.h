#ifndef ADCIRC_H
#define ADCIRC_H

#include <iostream>
#include <fstream>

#include "Point.h"
#include "Triangle.h"

#define BOGUS (-9999)


// typedef struct Triangle {
// 	Triangle(int v0=-1,int v1=-2,int v2=-3): v0(v0), v1(v1), v2(v2) {}
// 	Triangle(const Triangle &t): v0(t.v0), v1(t.v1), v2(t.v2) {}
// 	Triangle& operator=(const Triangle& other) = default;
// 	int v0, v1, v2;
// } Triangle;
// 

// reads the name of the grid given at the top of the file.
void readGridName(std::ifstream &in, std::string &name);


/** Read the specified number of vertices from the file into a vector. Each vertex is a pair of
	coordinates. This method adds a bogus vertex at the beginning of the vector to account for
	the 1-based indexing of vertices in the file. 
*/
std::vector<Point>  readVertices(std::ifstream &in, int numVertices);


/** Read the specified number of triangles from the file into a vector. Each triangle is a triplet of
	vertex indices.  
*/
std::vector<Triangle>  readTriangles(std::ifstream &in, int numTriangles);


#endif
