#ifndef ADCIRC_H
#define ADCIRC_H

#include <iostream>
#include <fstream>
#include <vector>

#include "point.h"
#include "triangle.h"

#define BOGUS (-9999)


// typedef struct triangle {
// 	triangle(int v0=-1,int v1=-2,int v2=-3): v0(v0), v1(v1), v2(v2) {}
// 	triangle(const triangle &t): v0(t.v0), v1(t.v1), v2(t.v2) {}
// 	triangle& operator=(const triangle& other) = default;
// 	int v0, v1, v2;
// } triangle;
// 

// reads the name of the grid given at the top of the file.
void readGridName(std::ifstream &in, std::string &name);


/** Read the specified number of vertices from the file into a vector. Each vertex is a pair of
	coordinates. This method adds a bogus vertex at the beginning of the vector to account for
	the 1-based indexing of vertices in the file. 
*/
std::vector<point>  readVertices(std::ifstream &in, int numVertices);



/** Read the specified number of triangles from the file into a vector. Each triangle is a triplet of
	vertex indices. Point values will be read from the points vector.
*/
std::vector<triangle>  readTriangles(std::ifstream &in, int numtriangles, std::vector<point> points);




/** Read the specified number of triangles from the file into a vector. Each triangle is a triplet of
	vertex indices. points in the vector will have valid Id fields, but invalid x and y coordinates.
*/
std::vector<triangle>  readTrianglesAsThreeIds(std::ifstream &in, int numtriangles);


#endif
