#include "gridElement.h"

#ifndef GRIDBOUND_H
#define GRIDBOUND_H

//////////////////////////////////////////////////////////////////////////////////////////////////////
//This is the definition of a grid box
// A gridbox includes low left corner, and high right corner partiton element
//////////////////////////////////////////////////////////////////////////////////////////////////////
class gridBound
{	
private:
	//low left corner partition
	gridElement lowGridElement;

	//high right corner partition
	gridElement highGridElement;
public:
		gridBound(gridElement lowGridElementInput, gridElement highGridElementInput);
		void setLowGridElement(gridElement gridElementInput);
		void setHighGridElement(gridElement gridElementInput);
		gridElement getLowGridElement();
		gridElement getHighGridElement();

	/* Count the number of intersections of a bounding box (around the cell) to the grid elements in geology bound*/
		int elementCount();
};

#endif
