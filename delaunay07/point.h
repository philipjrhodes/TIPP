#include <iostream>

#ifndef POINT_H
#define POINT_H

class point{
public:
	//coordinates of points in domain
	float x;
	float y;
	//order of points in domain
	unsigned int Id;

	//Constructors
	point();
	point(float xInput, float yInput, unsigned int id);
	point(float xInput, float yInput);
	point(const point &pInput);

	// Operations
	void set(point pInput);
	void setX(float xInput);
	void setY(float yInput);

	void setId(unsigned int id);
	unsigned int getId();

	float getX();
	float getY();
	bool operator == (point p);
};
inline std::ostream &operator << (std::ostream &str, point const &p){
	return str << "Point id: " << p.Id << " x= "<<p.x<<" y="<<p.y<<std::endl;
}

#endif

