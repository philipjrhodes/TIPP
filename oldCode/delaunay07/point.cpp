#include "point.h"

///////////////////////////////////////////////////////////////////////////////////////
/* 2D point with coordinate x and y*/
///////////////////////////////////////////////////////////////////////////////////////
point::point(float xInput, float yInput, unsigned int id){
	x = xInput;
	y = yInput;
	Id = id;
}

point::point(float xInput, float yInput){
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

float point::getX(){
	return x;
}

float point::getY(){
	return y;
}

void point::setX(float xInput){
	x = xInput;
}

void point::setY(float yInput){
	y = yInput;
}

void point::setId(unsigned int id){
	Id = id;
}
unsigned int point::getId(){
	return Id;
}

bool point::operator == (const point p){
	return (Id == p.Id);
}

//void point::print(){
//	std::cout<<"["<<x<<", "<<y<<"] ";
//}
