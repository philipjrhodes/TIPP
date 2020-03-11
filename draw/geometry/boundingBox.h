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
 
 inline boundingBox getBoundingBox(std::vector<boundingBox> const & quads){
 
    int length = quads.size();

    if (quads.empty()) 
        return boundingBox( 0,0, -1, -1); //invalid 
    
    point listlow = quads[0].getLowPoint();
    point listhi = quads[0].getHighPoint();
    
    double minx = listlow.getX();
    double miny = listlow.getY();
    
    double maxx = listhi.getX();
    double maxy = listhi.getY();
    
    for(int i=1; i<length; i++){
        
        listlow = quads[i].getLowPoint();
        listhi = quads[i].getHighPoint();
        
        minx = (listlow.getX() < minx) ? listlow.getX() : minx ;
        miny = (listlow.getY() < miny) ? listlow.getY() : miny ;
        
        maxx = (listhi.getX() > maxx) ? listhi.getX() : maxx ;
        maxy = (listhi.getY() > maxy) ? listhi.getY() : maxy ;
    }
    
    return boundingBox(minx, miny, maxx, maxy);

 }

#endif
