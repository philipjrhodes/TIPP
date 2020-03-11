#include "boundingBox.h"

////////////////////////////////////////////////////////////////////////////////////////////
/*the rectangle on the surface including lowLeft and higHright corners*/
////////////////////////////////////////////////////////////////////////////////////////////
 boundingBox::boundingBox(point lowPointInput, point highPointInput): lowPoint(lowPointInput), highPoint(highPointInput){}

 boundingBox::boundingBox(double lowX, double lowY, double highX, double highY){
 
 	setBox(lowX,lowY, highX, highY);
}

void boundingBox::setBox(double lowX, double lowY, double highX, double highY){

	lowPoint.setX(lowX);
	lowPoint.setY(lowY);
	highPoint.setX(highX);
	highPoint.setY(highY);
}

 void boundingBox::setLowPoint(point pointInput){
	lowPoint = pointInput;
}

 void boundingBox::setHighPoint(point pointInput){
	highPoint = pointInput;
}

const point boundingBox::getLowPoint() const {
	return lowPoint;
}

const point boundingBox::getHighPoint()const{
	return highPoint;
}


