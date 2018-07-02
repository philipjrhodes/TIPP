#include "gridElement.h"

////////////////////////////////////////////////////////////////////////////////////////////
//logical coordinates of a partition element
//partition a square geology into 2x2 --> (0,0), (0,1) (1,0), (1,1)
////////////////////////////////////////////////////////////////////////////////////////////
 gridElement::gridElement(int xInput, int yInput){
	gridx = xInput;
	gridy = yInput;
}

 gridElement::gridElement(const gridElement &gridElementInput){
//	gridx = gridElementInput.getX();
//	gridy = gridElementInput.getY();
	gridx = gridElementInput.gridx;
	gridy = gridElementInput.gridy;
}

 void gridElement::setX(int xInput){
	gridx = xInput;
}

 void gridElement::setY(int yInput){
	gridy = yInput;
}

 int gridElement::getX(){
	return gridx;
}

 int gridElement::getY(){
	return gridy;
}
