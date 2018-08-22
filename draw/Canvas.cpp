#include "Canvas.h"
#include <string.h>

Canvas::Canvas() {
  
  	// These defaults should be reset by child classes. 				
	this->lx = 0.0;
	this->ly = 0.0;
	
	this->xratio = 1.0;
	this->yratio = 1.0;
	this->margin = 5;
}

void Canvas::setMapping( double vminx, double vminy, double vmaxx, double vmaxy, double pageWidth, double pageHeight){

	this->lx = vminx;
	this->ly = vminy;

	this->xratio = (pageWidth  - 2 * (this->margin)) / (vmaxx - this->lx) ;
	this->yratio = (pageHeight - 2 * (this->margin)) / (vmaxy - this->ly) ;
	
// 	if(this->xratio < this->yratio){
// 	
// 		this->xratio = this->yratio;
// 	}
}

void Canvas::setMapping( double vminx, double vminy, double vmaxx, double vmaxy){

	this->lx = vminx;
	this->ly = vminy;
	
	double pageWidth  = this->getPageWidth();
    double pageHeight = this->getPageHeight();

	this->xratio = (pageWidth  - 2 * (this->margin)) / (vmaxx - this->lx) ;
	this->yratio = (pageHeight - 2 * (this->margin)) / (vmaxy - this->ly) ;
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
   				
