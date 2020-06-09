/*
 * edge.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */


#include "edge.h"

edge::edge(const point &pt1, const point &pt2){
	p1 = pt1;
	p2 = pt2;
}

edge::edge(const edge &e){
	p1 = e.p1;
	p2 = e.p2;
}

bool edge::operator == (const edge &e){
	return 	(((p1 == e.p1) && (p2 == e.p2)) ||
			((p1 == e.p2) && (p2 == e.p1)));
}



