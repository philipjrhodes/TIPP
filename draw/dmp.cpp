/* dmp.c
 *   Copyright (c) 2018 Philip J. Rhodes
 *
 * Derived from:
 * << Haru Free PDF Library 2.0.0 >> -- line_demo.c
 *
 * Copyright (c) 1999-2006 Takeshi Kanno <takeshi_kanno@est.hi-ho.ne.jp>
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.
 * It is provided "as is" without express or implied warranty.
 *
 */


//TODO: get rid of C stuff
// #include <stdlib.h>
// #include <stdio.h>  
// #include <string.h>
// #include <setjmp.h>

#include <iostream>
#include <fstream>
#include <vector>

#include "point.h"
#include "triangle.h"

#include "PDFCanvas.h"
#include "adcirc.h"


using namespace std;

int main (int argc, char **argv)
{	
	ifstream in("nc_inundation_v6c.grd");
	string name;
	int numVertices, numTriangles;

	readGridName(in, name); 
	in >> numTriangles;
	in >> numVertices;
	
	cout << "gridfile title:" << name << endl;
	cout << "numVertices=" << numVertices << "   numTriangles=" << numTriangles << endl;

	std::vector<point> v = readVertices(in, numVertices);
// 	std::vector<triangle> t = readTrianglesAsThreeIds(in, numTriangles, v);
 	std::vector<triangle> t = readTriangles(in, numTriangles, v);
   



	Canvas *c = new PDFCanvas(name);
 
 	c->setMapping(v);
 	
//  	c->setStrokeColor(1, 0, 0);
//  	c->setFillColor(0.9, 0.9, 1);
//  	c->setStrokeWidth(2.0);
//  	
//  	c->drawRect(200,600, 100,100);
//  	c->drawRect(200,500, 100,100);
//  	c->drawRect(100,500, 100,100);
//  	c->drawRect(100,600, 100,100);

	c->setStrokeWidth(0.01);
 	
//  	c->enableFill();
//  	c->drawCircle(300,400, 100);
//  	
//  	c->setStrokeColor(0, 0, 0);
//  	c->disableFill();

//  	c->drawTriangles(v, t);	
    c->drawTriangles(t);
    
	c->saveToFile("dmpdf.pdf");
	
	delete c;
    return 0;
}

