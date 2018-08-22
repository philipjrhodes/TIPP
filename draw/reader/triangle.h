/*
 * triangle.h
 *
 *  Created on: Oct 3, 2016
 *      Author: kevin
 */

#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "edge.h"
#include "boundingBox.h"
#include <vector>
#include <cmath>
#include <iostream>

typedef unsigned char BYTE;

double max3(double a, double b, double c);
double min3(double a, double b, double c);


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
    
    // These are different from the circumcircle methods.
    inline double getMinX() const { 
        return min3(p1.x, p2.x, p3.x);           
    }
    
    inline double getMaxX() const { 
        return max3(p1.x, p2.x, p3.x);           
    }
    
    inline double getMinY() const { 
        return min3(p1.y, p2.y, p3.y);           
    }
    
    inline double getMaxY() const { 
        return max3(p1.y, p2.y, p3.y);           
    }
    
    
    ~triangle(){
    
        //std::cerr  << "Triangle Destructor called." << std::endl;
    }
};

inline double max3(double a, double b, double c) {

    if ( a > b) {
        if(a > c)
            return a; // a>b, a>c
        else
            return c; // a>b, c>=a
    } else {  
        if ( b > c)
            return b; // b>=a, b>c
        else 
            return c; // b>=a, c>=b
    }
}

inline double min3(double a, double b, double c) {

    if ( a < b) {
        if(a < c)
            return a; // a<b, a<c
        else
            return c; // a<b, c<=a
    } else {  
        if ( b < c)
            return b; // b<=a, b<c
        else 
            return c; // b<=a, c<=b
    }
}


inline std::ostream &operator << (std::ostream &str, const triangle &t){
    return str << "Triangle:" << std::endl << t.p1 << std::endl << t.p2 << std::endl << t.p3 << std::endl;
}

// return a point that with coordinates consisting of the smallest x and y coords of any vertex in 
// the list of triangles.
// This point is not necessarily a vertex.
inline optional<point> getMinPoint(std::vector<triangle> const & triangles)
{
    if (triangles.empty())
            return optional<point>();

    point min = triangles[0].p1; 
    double minX, minY;
    
    for (unsigned i = 0; i < triangles.size(); ++i) { 
            
        minX = triangles[i].getMinX();
        minY = triangles[i].getMinY();
        
        if (minX < min.x)
                min.x = minX;
                
        if (minY < min.y)
                min.y = minY;
    }
    
    return optional<point>(min);
}

// return a point that with coordinates consisting of the largest x and y coords of any vertex in 
// the list of triangles.
// This point is not necessarily a vertex.
inline optional<point> getMaxPoint(std::vector<triangle> const & triangles)
{
    if (triangles.empty())
            return optional<point>();

    point max = triangles[0].p1; 
    double maxX, maxY;
    
    for (unsigned i = 0; i < triangles.size(); ++i) { 
            
        maxX = triangles[i].getMaxX();
        maxY = triangles[i].getMaxY();
        
        if (maxX > max.x)
                max.x = maxX;
                
        if (maxY > max.y)
                max.y = maxY;
    }
    
    return optional<point>(max);
}

//TODO: add boundingBox method for triangle vector

#endif
