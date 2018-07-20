
/*******************************************************************************
*  The "New BSD License" : http://www.opensource.org/licenses/bsd-license.php  *
********************************************************************************

Copyright (c) 2010, Mark Turney
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/
#include<iostream>
#include<fstream>

#include "simple_svg_1.0.0.hpp"
#include "assert.h"

using namespace svg;
using namespace std;

// Demo page shows sample usage of the Simple SVG library.

typedef struct Triangle {
	Triangle(int v0=-1,int v1=-2,int v2=-3): v0(v0), v1(v1), v2(v2) {}
//	Triangle(): v0(0), v1(0), v2(0) {}
	Triangle(const Triangle &t): v0(t.v0), v1(t.v1), v2(t.v2) { /*cerr << "called cc:" << endl;*/ v0=t.v0; v1=t.v1; v2=t.v2;}
	Triangle& operator=(const Triangle& other) = default;
	int v0, v1, v2;

} Triangle;

void readGridName(ifstream &in, string &name){

	getline(in, name);
}

vector<Point>  readVertices(ifstream &in, int numVertices){

	vector<Point> *v = new vector<Point>();
//	v->reserve(numVertices);
	int l;
	float x,y,z;
	float vScale=16, dx=80, dy=-20;
	
	Point p(-13,-17);
	
	for(int i=0; i<numVertices; i++){
	
		in >> l >> x >> y >> z;
		assert(l == (i+1));
		
		x = (x+dx)*vScale;
		y = (y+dy)*vScale;
//		y = y*vScale + dy;
		
		p.x=x; p.y=y;
		v->push_back(p);
	}
	cout << "last vertex is numbered " << l << " in the file." << endl;
	
	return *v;
}

vector<Triangle>  readTriangles(ifstream &in, int numTriangles){

	vector<Triangle> *v = new vector<Triangle>();
//	v->reserve(numTriangles);
	int l,d,v0,v1,v2;
	Triangle t(-1, -1, -1);
	
	for(int i=0; i<numTriangles; i++){
	
		in >> l >> d >> v0 >> v1 >> v2;
		assert(l == (i+1));
		assert(d == 3);
		t.v0=v0; t.v1=v1; t.v2=v2;
		v->push_back(t);
	}
	
	cout << "readTriangles(): last triangle is numbered " << l << " in the file." << endl;
	cout << "readTriangles(): triangle vector has size: " << v->size() << endl;
	return *v;
}


void drawTriangles(Document &doc,const vector<Point> &vertices, const vector<Triangle> &triangles){

//	int numVertices = vertices.size();
	int numTriangles = triangles.size();
	
	cerr<< "drawTriangles(): numTriangles == " << numTriangles << endl;
	for(int i=0; i< numTriangles; i++){
			
// 		cerr  << "v0: " << triangles[i].v0 << " v1: "<< triangles[i].v1 << " v2: " << triangles[i].v2;
// 		cerr  << "\t\tx0: " << vertices[triangles[i].v0].x << " x1: " << vertices[triangles[i].v1].x << " x2: " << vertices[triangles[i].v2].x  << endl;	
		
		doc << (Polygon(Color::White, Stroke(.5, Color::Blue)) << vertices[triangles[i].v0] << vertices[triangles[i].v1] << vertices[triangles[i].v2]) ;	
		//cout << i << endl;
	}
}


int main()
{
    Dimensions dimensions(400, 400);
    Document doc("nc_inundation_v6c.grd.svg", Layout(dimensions, Layout::TopLeft, 1, Point(dimensions.width/2, dimensions.height/2)));
    
    ifstream in("nc_inundation_v6c.grd");

    // Red image border.
    Polygon border(Stroke(1, Color::Red));
    border <<  Point(-dimensions.width/2, -dimensions.height/2) << Point(-dimensions.width/2, dimensions.height/2) << Point(dimensions.width/2, dimensions.height/2) << Point(dimensions.width/2, -dimensions.height/2);

	//border << Point(-100, -100) << Point(-100, 100) << Point(100, 100) << Point(100, -100);

    // Green Origin.
    Rectangle origin(Point(0,0), 1, 1, Fill(Color::Blue), Stroke(1, Color::Green));
    

    
//     border << Point(0, 0) << Point(dimensions.width, 0) << Point(dimensions.width, dimensions.height) << Point(0, dimensions.height);

	doc << origin;
    doc << border;

 
// 	Polyline triangle(Stroke(.5, Color::Blue));
// 	triangle << Point(0, 0) << Point(90, 30) << Point(50, 90) << Point(0, 0);
// 	doc << triangle;
	
	string name;
	int numVertices, numTriangles;

	readGridName(in, name);	
	in >> numTriangles;
	in >> numVertices;
	
	cout << "gridfile title:" << name << endl;
	cout << "numVertices=" << numVertices << "   numTriangles=" << numTriangles << endl;
	
	vector<Point> v = readVertices(in, numVertices);
//	cout << v[10].x << endl;

	vector<Triangle> t = readTriangles(in, numTriangles);
//	cout << t[10].v1 << endl;
	
	drawTriangles(doc, v, t);
	
	

//     //Condensed notation, parenthesis isolate temporaries that are inserted into parents.
//     doc << (LineChart(Dimensions(65, 5))
//         << (Polyline(Stroke(.5, Color::Blue)) << Point(0, 0) << Point(10, 8) << Point(20, 13))
//         << (Polyline(Stroke(.5, Color::Orange)) << Point(0, 10) << Point(10, 16) << Point(20, 20))
//         << (Polyline(Stroke(.5, Color::Cyan)) << Point(0, 5) << Point(10, 13) << Point(20, 16)));
// 
//     doc << Circle(Point(80, 80), 20, Fill(Color(100, 200, 120)), Stroke(1, Color(200, 250, 150)));
// 
//     doc << Text(Point(5, 77), "Simple SVG", Color::Silver, Font(10, "Verdana"));
// 
//     doc << (Polygon(Color(200, 160, 220), Stroke(.5, Color(150, 160, 200))) << Point(20, 70)
//         << Point(25, 72) << Point(33, 70) << Point(35, 60) << Point(25, 55) << Point(18, 63));
// 
//     doc << Rectangle(Point(70, 55), 20, 15, Color::Yellow);

    doc.save();
}



