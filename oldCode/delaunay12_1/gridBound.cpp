#include "gridBound.h"
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////
//This is the definition of a grid box
// A gridbox includes low left corner, and high right corner partiton element
////////////////////////////////////////////////////////////////////////////////////////////

gridBound::gridBound(gridElement lowGridElementInput, gridElement highGridElementInput):
				lowGridElement(lowGridElementInput), highGridElement(highGridElementInput){}

void gridBound::setLowGridElement(gridElement gridElementInput){
	lowGridElement = gridElementInput;
}

void gridBound::setHighGridElement(gridElement gridElementInput){
	highGridElement = gridElementInput;
}

gridElement gridBound::getLowGridElement(){
	return lowGridElement;
}

gridElement gridBound::getHighGridElement(){
	return highGridElement;
}


/* Count the number of intersections of a bounding box (around the cell) to the grid elements in geology bound*/
int gridBound::elementCount(){
	/*the coordiante of two corners of bounding grid*/
	int beginx = lowGridElement.getX();
	int beginy = lowGridElement.getY();
	int endx = highGridElement.getX();
	int endy = highGridElement.getY();
//std::cout<<beginx<<" "<<endx<<" "<<beginy<<" "<<endy<<"\n";
	return (endy - beginy + 1) * (endx - beginx + 1);
}


