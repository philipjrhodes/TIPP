#include "point.h"

///////////////////////////////////////////////////////////////////////////////////////
/* 2D point with coordinate x and y*/
///////////////////////////////////////////////////////////////////////////////////////
point::point(double xInput, double yInput, unsigned long long int id){
	x = xInput;
	y = yInput;
	Id = id;
}

point::point(double xInput, double yInput){
	x = xInput;
	y = yInput;
}

point::point(){
}

point::point(const point &pInput){
	x = pInput.x;
	y = pInput.y;
	Id = pInput.Id;
}

void point::set(point pInput){
	x = pInput.x;
	y = pInput.y;
	Id = pInput.Id;

}

double point::getX(){
	return x;
}

double point::getY(){
	return y;
}

void point::setX(double xInput){
	x = xInput;
}

void point::setY(double yInput){
	y = yInput;
}

void point::setId(unsigned long long int id){
	Id = id;
}
unsigned long long int point::getId(){
	return Id;
}

point& point::operator = (const point& p){
	x = p.x;
	y = p.y;
	Id = p.Id;
	return *this;
}

bool point::operator == (const point p){
	return (Id == p.Id);
}

//void point::print(){
//	std::cout<<"["<<x<<", "<<y<<"] ";
//}
