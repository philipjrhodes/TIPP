#include <iostream>

#ifndef POINT_H
#define POINT_H

class point{
public:
	//coordinates of points in domain
	double x;
	double y;
	//order of points in domain
	unsigned long long int Id;

	//Constructors
	point();
	point(double xInput, double yInput, unsigned long long int id);
	point(double xInput, double yInput);
	point(const point &pInput);

	// Operations
	void set(point pInput);
	void setX(double xInput);
	void setY(double yInput);

	void setId(unsigned long long int id);
	unsigned long long int getId();

	double getX();
	double getY();
	bool operator == (point p);
};
inline std::ostream &operator << (std::ostream &str, point const &p){
	return str << "Point id: " << p.Id << " x= "<<p.x<<" y="<<p.y<<std::endl;
}

#endif

