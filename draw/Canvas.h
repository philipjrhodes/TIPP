#ifndef CANVAS_H
#define CANVAS_H
#include <string>
#include "Point.h"
#include "Triangle.h"

/* A Canvas is an abstraction for a graphics file.
	window space -- rectangular subregion of vertex space
	page space   -- space containing page's rectangular bounds 
*/

class Canvas {
   public:
   		
   		Canvas();

		// Set the mapping between the window (vertex) space and page space.
     	void setMapping( double vminx, double vminy, double vmaxx, double vmaxy, double pageWidth, double pageHeight);
        
        // Set the mapping between the window space and page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // in the given list of vertices.
        void setMapping(const std::vector<Point> &vertices);
        
        virtual inline void enableFill(){ drawStyle |= FILL;};
        
        virtual void disableFill(){ drawStyle &= ~FILL;};
        
        virtual void enableStroke() { drawStyle = STROKE;};
        
        virtual void disableStroke(){ drawStyle &= ~STROKE;};
        

		// Color used for interior of shapes.
    	virtual void setFillColor(double r, double g, double b)=0;
    	
    	// Color used for outline of shapes.
    	virtual void setStrokeColor(double r, double g, double b)=0;
 
 		//Width of lines
 		virtual void setStrokeWidth(double w)=0;  		
    		
		virtual void drawTriangle(
			double        x0,
			double        y0,
			double        x1,
			double        y1,
			double        x2,
			double        y2) = 0;
		
		virtual void drawCircle(
			double cx,
			double cy,
			double r) = 0;
			
		virtual void drawRect(
			double xmin,
			double ymin,
			double width,
			double height)=0;

		virtual void drawTriangles(
			const std::vector<Point> &vertices,
			const std::vector<Triangle> &triangles)=0;
         
       
        virtual double getPageWidth()  = 0;
        virtual double getPageHeight() = 0;
        
        virtual int saveToFile(std::string filename) = 0;        
        virtual ~Canvas(){};
    
    protected:
    
    	double lx, ly, xratio, yratio, margin;
    	std::string filename;
		
		enum DRAWSTYLE {FILL=1, STROKE=2};

		int drawStyle=STROKE;
    	

		void mapToPage(double x, double y, double &px, double &py);
		
		int hasCorrectExtension(const char * s1, const char * extension);
		
		virtual void copyNameWithExtension(std::string fname, std::string extension);
				
		
	private:

		


};

#endif
