/*
 * triangle.h
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */

#ifndef H_TRIANGLE
#define H_TRIANGLE

#include "edge.h"
#include "boundingBox.h"
#include <cmath>

typedef unsigned char BYTE;

class triangle{
public:
	point p1;
	point p2;
	point p3;
	double centerX;
	double centerY;
	double radius;

	//not delivered yet, will be dilivered for a partition
	bool delivered;

	triangle(const point &_p1, const point &_p2, const point &_p3);
	triangle(const triangle &t);
	triangle(){}

	edge getEdge1();
	edge getEdge2();
	edge getEdge3();
	bool operator == (const triangle &t);
	bool colinear();
	bool isBad();
	double area();
	bool containsVertex(const point &p);
	double getFarestCoorX();
	double getNearestCoorX();
	double getHighestCoorY();
	double getLowestCoorY();

	//Each point has an id
	void computeCenterRadius();
	bool circumCircleContains(point p);

	double distance(point p1, point p2);
	BYTE outCode(point lowPoint, point highPoint, point p);
	//check intersection between a rectangle (partition) and current triangle
	//refenrence: https://yal.cc/rectangle-circle-intersection-test/
	bool intersect(boundingBox bBox);
	bool inside(boundingBox bBox);
	double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3);
};

inline std::ostream &operator << (std::ostream &str, const triangle &t){
	return str << "Triangle:" << std::endl << t.p1 << std::endl << t.p2 << std::endl << t.p3 << std::endl;
}
#endif
