#include<iostream>
#include<fstream>
#include <assert.h>

#include "adcirc.h"

using namespace std;
//using namespace svg;

// reads the name of the grid given at the top of the file.
void readGridName(ifstream &in, string &name){

	getline(in, name);
}


/** Read the specified number of vertices from the file into a vector. Each vertex is a pair of
	coordinates. This method adds a bogus vertex at the beginning of the vector to account for
	the 1-based indexing of vertices in the file. 
*/
vector<Point>  readVertices(ifstream &in, int numVertices){

	vector<Point> *v = new vector<Point>();
//	v->reserve(numVertices);
	int l;
	float x,y,z;
//	float vScale=16, dx=80, dy=-20;
	
	Point p(BOGUS,BOGUS);
	v->push_back(p); // compensate for 1-based vertex indexing in file.
	
	for(int i=0; i<numVertices; i++){
	
		in >> l >> x >> y >> z;
		assert(l == (i+1));
		
//		x = (x+dx)*vScale;
//		y = (y+dy)*vScale;
//		y = y*vScale + dy;
		
		p.x = x; p.y = y;
		v->push_back(p);
	}
	cout << "last vertex is numbered " << l << " in the file." << endl;
	
	return *v;
}

/** Read the specified number of triangles from the file into a vector. Each triangle is a triplet of
	vertex indices.  
*/
vector<Triangle>  readTriangles(ifstream &in, int numTriangles){

	vector<Triangle> *v = new vector<Triangle>();
//	v->reserve(numTriangles);
	int l,n,v0,v1,v2;
	Triangle t(BOGUS, BOGUS, BOGUS);
	
	for(int i=0; i<numTriangles; i++){
	
		in >> l >> n >> v0 >> v1 >> v2;
		assert(l == (i+1));
		assert(n == 3);

		//t.v0=v0-1; t.v1=v1-1; t.v2=v2-1;
		t.v0=v0; t.v1=v1; t.v2=v2;
		v->push_back(t);
	}
	
	cout << "readTriangles(): last triangle is numbered " << l << " in the file." << endl;
	cout << "readTriangles(): triangle vector has size: " << v->size() << endl;
	return *v;
}




