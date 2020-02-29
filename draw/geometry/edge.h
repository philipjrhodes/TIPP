/*
 * edge.h
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */

#ifndef H_EDGE
#define H_EDGE

#include <iostream>
#include "point.h"

class edge{
public:
	point p1;
	point p2;

	edge(const point &pt1, const point &pt2);
	edge(const edge &e);
	edge(){}
	bool operator == (const edge &e);
};

inline std::ostream &operator << (std::ostream &str, const edge &e){
	return str << "Edge:" << std::endl << e.p1 << std::endl << e.p2 << std::endl;
}
#endif
