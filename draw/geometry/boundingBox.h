#include "point.h"

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

//////////////////////////////////////////////////////////////////////////////////////////////////////
/*the rectangle on the surface including lowLeft and higHright corners*/
//////////////////////////////////////////////////////////////////////////////////////////////////////
class boundingBox
{
private:
	//low left corner coordinates
	point lowPoint;

	//high right corner coordinates
	point highPoint;

public:
	 boundingBox(point lowPointInput, point highPointInput);
	 boundingBox(double lowX, double lowY, double highX, double highY);
	 boundingBox(){};
	 
	 void setBox(double lowX, double lowY, double highX, double highY);

	 void setLowPoint(point pointInput);
	 void setHighPoint(point pointInput);
	 const point getLowPoint() const;
	 const point getHighPoint() const;
};

#endif
