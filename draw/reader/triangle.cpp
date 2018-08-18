/*
 * triangle.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */


#include "triangle.h"
#include "edge.h"

#include <assert.h>
#include <limits>
#include <math.h>

double max = std::numeric_limits<double>::max();
double inf = std::numeric_limits<double>::infinity();


triangle::triangle(const point &_p1, const point &_p2, const point &_p3){
	p1 = _p1;
	p2 = _p2;
	p3 = _p3;
	//not delivered yet, will be dilivered for a partition
	delivered = false;
	computeCenterRadius();
}

triangle::triangle(const triangle &t){
	p1 = t.p1;
	p2 = t.p2;
	p3 = t.p3;
	delivered = false;
	computeCenterRadius();	
}

edge triangle::getEdge1(){
	return edge(p1, p2);
}
edge triangle::getEdge2(){
	return edge(p2, p3);
}
edge triangle::getEdge3(){
	return edge(p3, p1);
}

bool triangle::containsVertex(const point &p){
	return ((p1 == p) || (p2 == p) || (p3 == p));
}

bool triangle::operator == (const triangle &t){
	return (p1 == t.p1 || p1 == t.p2 || p1 == t.p3) &&
				(p2 == t.p1 || p2 == t.p2 || p2 == t.p3) &&
				(p3 == t.p1 || p3 == t.p2 || p3 == t.p3);
}


bool triangle::colinear(){
	double epsilon = 1e-15;
	double x1 = p1.getX();
	double y1 = p1.getY();

	double x2 = p2.getX();
	double y2 = p2.getY();

	double x3 = p3.getX();
	double y3 = p3.getY();
	return (fabs(x1*(y2-y3)+x2*(y3-y1)+x3*(y1-y2))<epsilon);
}

//==============================================================
//check if centerX ot cemterY, or radius have value inf (infinity)
bool triangle::isBad(){
	return ((centerX==inf)||(centerX==-inf)||(centerY==inf)||(centerY==-inf)||(radius==inf)||(radius==-inf));
}

//==============================================================
double triangle::area(){
	double x1 = p1.getX();
	double y1 = p1.getY();

	double x2 = p2.getX();
	double y2 = p2.getY();

	double x3 = p3.getX();
	double y3 = p3.getY();

	return (x1*(y2-y3) + x2*(y3-y1) +x3*(y1-y2))/2.0;
}


//determine if a point stay inside the circle of triangle
//http://math.stackexchange.com/questions/213658/get-the-equation-of-a-circle-when-given-3-points
//http://mathworld.wolfram.com/Circumcircle.html
//http://www.qc.edu.hk/math/Advanced%20Level/circle%20given%203%20points.htm
//http://www.regentsprep.org/regents/math/geometry/gcg6/RCir.htm
void triangle::computeCenterRadius(){
	double x1 = p1.getX();
	double y1 = p1.getY();

	double x2 = p2.getX();
	double y2 = p2.getY();

	double x3 = p3.getX();
	double y3 = p3.getY();

	double ab = (x1 * x1) + (y1 * y1);
	double cd = (x2 * x2) + (y2 * y2);
	double ef = (x3 * x3) + (y3 * y3);

	centerX =(double)(ab * (y3 - y2) + cd * (y1 - y3) + ef * (y2 - y1)) / (double)(x1 * (y3 - y2) + x2 * (y1 - y3) + x3 * (y2 - y1)) / double(2.0);
	centerY = (double)(ab * (x3 - x2) + cd * (x1 - x3) + ef * (x2 - x1)) / (double)(y1 * (x3 - x2) + y2 * (x1 - x3) + y3 * (x2 - x1)) / double(2.0);
	radius = sqrt(((x1 - centerX) * (x1 - centerX)) + ((y1 - centerY) * (y1 - centerY)));
}

bool triangle::circumCircleContains(point p){

	double x = p.getX();
	double y = p.getY();
	
	double dist = sqrt(((x - centerX) * (x - centerX)) + ((y - centerY) * (y - centerY)));

	return dist < radius;
}

//on the right of circumcircle
double triangle::getFarestCoorX(){
	return centerX + radius;
}

//on the left of circumcircle
double triangle::getNearestCoorX(){
	return centerX - radius;
}

//on the top of circumcircle
double triangle::getHighestCoorY(){
	return centerY + radius;
}

//on the bottom of circumcircle
double triangle::getLowestCoorY(){
	return centerY - radius;
}

/*	1001	1000	1010
	0001	0000	0010
	0101	0100	0110
*/
BYTE triangle::outCode(point lowPoint, point highPoint, point p){
	point lowerPoint(lowPoint);
	point upperPoint(highPoint);
	float x = p.getX();
	float y = p.getY();

	BYTE b = (((y > highPoint.getY()?1:0) << 3) |
			((y < lowPoint.getY()?1:0) << 2) |
			((x > highPoint.getX()?1:0) << 1) |
			((x < lowPoint.getX()?1:0)));
	return b;	
}

//distance between two points
double triangle::distance(point p1, point p2){
	return sqrt( (p1.getX()-p2.getX())*(p1.getX()-p2.getX()) + (p1.getY()-p2.getY())*(p1.getY()-p2.getY()) );
}

//check intersection between a rectangle (partition) and current triangle's circumcircle
//refenrence: https://yal.cc/rectangle-circle-intersection-test/
bool triangle::intersect(boundingBox bBox){
	point lowPoint = bBox.getLowPoint();
	point highPoint = bBox.getHighPoint();
	point center = point(centerX, centerY);

	double d;//distance
	BYTE code = outCode(lowPoint, highPoint, center);
	switch (code){
		//center of circle stays inside rectangle
		case 0x00: return true; break;

		//center of circle stays on top or bottom of rectangle
		case 0x08: d = distance(point(highPoint.getX(), centerY), highPoint);break;
		case 0x04: d = distance(point(lowPoint.getX(), centerY), lowPoint);break;

		//center of circle stays on left or right of rectangle
		case 0x01: d = distance(point(centerX, lowPoint.getY()), lowPoint);break;
		case 0x02: d = distance(point(centerX, highPoint.getY()), highPoint);break;

		//center of circle stays on outside of low coner or outside of right coner of rectangle
		case 0x05: d = distance(point(centerX, centerY), lowPoint);break;
		case 0xA: d = distance(point(centerX, centerY), highPoint);break;

		//center of circle stays on outside of high left coner or outside of right low coner of rectangle
		case 0x06: d = distance(center, point(highPoint.getX(), lowPoint.getY()) );break;
		case 0x09: d = distance(center, point(lowPoint.getX(), highPoint.getY()) );
	}

	if(d<=radius) return true;
	else return false;
}

//check the circumcircle of this triangle stay inside the boundingbox or not
bool triangle::inside(boundingBox bBox){
	//boundingbox
	double farBoundingBoxX = bBox.getHighPoint().getX();
	double nearBoundingBoxX = bBox.getLowPoint().getX();
	double topBoundingBoxY = bBox.getHighPoint().getY();
	double bottomBoundingBoxY = bBox.getLowPoint().getY();

	//circumcircle
	double farCircleX = getFarestCoorX();
	double nearCircleX = getNearestCoorX();
	double topCircleY = getHighestCoorY();
	double bottomCircleY = getLowestCoorY();

	return ( (farCircleX<=farBoundingBoxX)&&(nearCircleX>=nearBoundingBoxX)&&(topCircleY<=topBoundingBoxY)&&(bottomCircleY>=bottomBoundingBoxY) );
}
