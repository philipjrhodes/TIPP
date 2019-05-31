#include <iostream>
#include <fstream>
#include <vector>
#include <assert.h>

#include "adcirc.h"
#include "point.h"
#include "triangle.h"

using namespace std;
//using namespace svg;

// reads the name of the grid given at the top of the file.
void readGridName(ifstream &in, string &name){

	getline(in, name);
}


/** Read the specified number of vertices from the file into a vector. Each vertex is a pair of
	coordinates. 
*/
vector<point>  readVertices(ifstream &in, int numVertices){

	vector<point> *v = new vector<point>(); //TODO: just declare as local. copy constructor should handle it.
//	v->reserve(numVertices);
	int l;
	float x,y,z;
	
	point p(BOGUS,BOGUS);
	//v->push_back(p); // compensate for 1-based vertex indexing in file. TODO: fix this.
	
	for(int i=0; i<numVertices; i++){
	
		in >> l >> x >> y >> z;
		assert(l == (i+1));
		
		
		p.x = x; p.y = y;
		v->push_back(p);
	}
	cout << "last vertex is numbered " << l << " in the file." << endl;
	
	return *v;
}

/** Read the specified number of triangles from the file into a vector. Each triangle is a triplet of
	vertex indices.  
*/
// vector<triangle>  readTriangles(ifstream &in, int numtriangles){
// 
// 	vector<triangle> *v = new vector<triangle>();
// //	v->reserve(numtriangles);
// 	int l,n,v0,v1,v2;
// //	triangle t(BOGUS, BOGUS, BOGUS);
// 	triangle t;
// 	
// 	for(int i=0; i<numtriangles; i++){
// 	
// 		in >> l >> n >> v0 >> v1 >> v2;
// 		assert(l == (i+1));
// 		assert(n == 3);
// 
// 		//t.v0=v0-1; t.v1=v1-1; t.v2=v2-1;
// 		//t.v0=v0; t.v1=v1; t.v2=v2;
// 		
// 		
// 		v->push_back(t);
// 	}
// 	
// 	cout << "readtriangles(): last triangle is numbered " << l << " in the file." << endl;
// 	cout << "readtriangles(): triangle vector has size: " << v->size() << endl;
// 	return *v;
// }


/** Read the specified number of triangles from the file into a vector. Each triangle in the file   
    is a triplet of indices, used to index the points vector. The returned vector consists of triangles
    with valid points.
*/
vector<triangle>  readTriangles(ifstream &in, int numtriangles, vector<point> points){

	vector<triangle> *v = new vector<triangle>();
	int l,n,v0,v1,v2;
	triangle t;
	
	for(int i=0; i<numtriangles; i++){
	
		in >> l >> n >> v0 >> v1 >> v2;
		assert(l == (i+1));
		assert(n == 3);

		//t.v0=v0-1; t.v1=v1-1; t.v2=v2-1;
		//t.v0=v0; t.v1=v1; t.v2=v2;
		
		t.p1 = points[v0-1]; t.p1.Id = v0-1;
		t.p2 = points[v1-1]; t.p2.Id = v1-1;
		t.p3 = points[v2-1]; t.p3.Id = v2-1;
		
		v->push_back(t);
	}
	
	cout << "readtriangles(): last triangle is numbered " << l << " in the file." << endl;
	cout << "readtriangles(): triangle vector has size: " << v->size() << endl;
	return *v;
}

/** Read the specified number of triangles from the file into a vector. Each triangle in the file 
    is a triplet of vertex indices. points in the vector will have valid Id fields, but invalid x and y coordinates.
*/
vector<triangle>  readTrianglesAsThreeIds(ifstream &in, int numtriangles){

	vector<triangle> *v = new vector<triangle>();
	int l,n,v0,v1,v2;

	triangle t;
	
	for(int i=0; i<numtriangles; i++){
	
		in >> l >> n >> v0 >> v1 >> v2;
		assert(l == (i+1));
		assert(n == 3);
        
		t.p1.Id = v0-1;
		t.p2.Id = v1-1;
		t.p3.Id = v2-1;
	
		v->push_back(t);
	}
	
	cout << "readTrianglesAsThreeIds(): last triangle is numbered " << l << " in the file." << endl;
	cout << "readTrianglesAsThreeIds(): triangle vector has size: " << v->size() << endl;
	return *v;
}



