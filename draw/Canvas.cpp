#include "Canvas.h"
#include <string.h>

Canvas::Canvas(int flipYAxis): windowMin(1,1), windowMax(-1,-1) { // max and min should initially be out of order.
//Canvas::Canvas(int flipYAxis): windowMin(0,0), windowMax(1,1) { 
  
  	// These defaults should be reset by child classes. 				
	this->lx = 0.0;
	this->ly = 0.0;
	
	this->xratio = 1.0;
	this->yratio = 1.0;
	this->margin = 15;
		
	this->yAxisFlipped = flipYAxis;
	
}

void Canvas::setMapping( double vminx, double vminy, double vmaxx, double vmaxy, double pageWidth, double pageHeight){

	this->lx = vminx;
	this->ly = vminy;

	this->xratio = (pageWidth  - 2 * (this->margin)) / (vmaxx - this->lx) ;
	this->yratio = (pageHeight - 2 * (this->margin)) / (vmaxy - this->ly) ;
	
	if(this->xratio > this->yratio){
	
		this->xratio = this->yratio;
	} else {
	
	    this->yratio = this->xratio;
	}
	
	if(this->yAxisFlipped){
	
	    this->ly = vmaxy;
	    this->yratio *= -1;
	}
}

void Canvas::setMapping( double vminx, double vminy, double vmaxx, double vmaxy){

	this->lx = vminx;
	this->ly = vminy;
	
	double pageWidth  = this->getPageWidth();
    double pageHeight = this->getPageHeight();

	this->xratio = (pageWidth  - 2 * (this->margin)) / (vmaxx - this->lx) ;
	this->yratio = (pageHeight - 2 * (this->margin)) / (vmaxy - this->ly) ;
	
	if(this->yAxisFlipped){
	
	    this->ly = vmaxy;
	    this->yratio *= -1;
	}

}


void Canvas::setMapping(const std::vector<point> &vertices){

	optional<point> min = getMinPoint(vertices);
	optional<point> max = getMaxPoint(vertices);
	
	printf("Canvas::setMapping(vector<point>):min.x = %lf, min.y= %lf  max.x = %lf, max.y= %lf\n",min->x, min->y, max->x, max->y);
	
    double width=this->getPageWidth();
    double height=this->getPageHeight();
	
	setMapping(min->x, min->y, max->x, max->y, width, height);
}


void Canvas::setMapping(const std::vector<triangle> &triangles){

	optional<point> min = getMinPoint(triangles); //TODO: add boundingBox method for triangle vector
	optional<point> max = getMaxPoint(triangles);
	
	printf("Canvas::setMapping(vector<triangle>):min.x = %lf, min.y= %lf  max.x = %lf, max.y= %lf\n",min->x, min->y, max->x, max->y);
	
    double width=this->getPageWidth();
    double height=this->getPageHeight();
	
	setMapping(min->x, min->y, max->x, max->y, width, height);
}


void Canvas::updateMapping(const std::vector<triangle> &triangles){

	optional<point> min = getMinPoint(triangles); //TODO: add boundingBox method for triangle vector
	optional<point> max = getMaxPoint(triangles);
	
	if(this->windowMin.x >  this->windowMax.x){ // first call for this method.
	
	    this->windowMin = min.unbox();
	    this->windowMax = max.unbox();
	} else {
	    
	    this->windowMin.x = (min->x < this->windowMin.x) ? min->x : this->windowMin.x;
	    this->windowMin.y = (min->y < this->windowMin.y) ? min->y : this->windowMin.y;	    
	    
	    this->windowMax.x = (max->x > this->windowMax.x) ? max->x : this->windowMax.x;
	    this->windowMax.y = (max->y > this->windowMax.y) ? max->y : this->windowMax.y;
	}
	
	printf("Canvas::updateMapping(vector<triangle>):min->x = %lf, min->y= %lf  max->x = %lf,  max->y= %lf\n",min->x, min->y, max->x, max->y);	
	printf("Canvas::updateMapping(vector<triangle>):windowMin.x = %lf, windowMin.y= %lf  windowMax.x = %lf, windowMax.y= %lf\n",windowMin.x, windowMin.y, windowMax.x, windowMax.y);
	
    double width=this->getPageWidth();
    double height=this->getPageHeight();
	
	setMapping(windowMin.x, windowMin.y, windowMax.x, windowMax.y, width, height);
}

void Canvas::updateMapping(const std::vector<triangle> * triangles){
    
    this->updateMapping( *triangles);
}


void Canvas::mapToPage(double x, double y, double &px, double &py){

	px = margin + (x - this->lx) * xratio;
	py = margin + (y - this->ly) * yratio;
}

int Canvas::hasCorrectExtension(const char * s1, const char * extension){

    int len = strlen(s1);
    int extLen = strlen(extension);
        
    return 0 == strncmp(s1 + len - extLen, extension, extLen); 
}

void Canvas::copyNameWithExtension(const std::string fname, const std::string extension){

	this->filename = fname;
	
	if (!hasCorrectExtension(filename.c_str(), extension.c_str())) {
		this->filename += extension;
	}
}
   				
