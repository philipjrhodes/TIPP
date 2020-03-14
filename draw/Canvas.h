#ifndef CANVAS_H
#define CANVAS_H

#include <string>
#include "point.h"
#include "triangle.h"

/* A Canvas is an abstraction for a graphics file.
	window space -- rectangular subregion of vertex space
	page space   -- space containing page's rectangular bounds 
*/

class Canvas {
   public:
   		
   		// Constructor. If flipYAxis is true, then the vertex space origin will map to
   		// the upper left of the page.
   		Canvas(int flipYAxis);

		// Set the mapping between the window (vertex) space and page space.
     	void setMapping( double vminx, double vminy, double vmaxx, double vmaxy, double pageWidth, double pageHeight);
        
        // Set the mapping between the window (vertex) space and default page space.
        void setMapping( double vminx, double vminy, double vmaxx, double vmaxy);
        
        
        // Set the mapping between the window space and default page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // in the given list of vertices.
        void setMapping(const std::vector<point> &vertices);

        // Set the mapping between the window space and page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // of the points contained directly in the triangles.
        void setMapping(const std::vector<triangle> &triangles);
        
        // Update the mapping between the window space and page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // of the points contained directly in the triangles, as well as the
        // max and min values resulting from previous calls to this method. 
        void updateMapping(const std::vector<triangle> &triangles);
        void updateMapping(const std::vector<triangle> *triangles);
 
        // Update the mapping between the window space and page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // of the points contained directly in the quads, as well as the
        // max and min values resulting from previous calls to this method. 
        void updateMapping(const std::vector<boundingBox> &quads);
        void updateMapping(const std::vector<boundingBox> *quads);

         // Update the mapping between the window space and page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // of the points contained directly in the boundingBox, as well as the
        // max and min values resulting from previous calls to this method. 
        void updateMapping(boundingBox box);
 
        // Update the mapping between the window space and page space. Computes
        // the window bounds by finding min and max vertex x and y coordinates
        // of the argument points, as well as the
        // max and min values resulting from previous calls to this method. 
        void updateMapping(const point min, const point max);
 
        
        
        virtual inline void enableFill(){ drawAction |= FILL;};
        
        virtual void disableFill(){ drawAction &= ~FILL;};
        
        virtual void enableStroke() { drawAction |= STROKE;};
        
        virtual void disableStroke(){ drawAction &= ~STROKE;};
        
        

		// Color used for interior of shapes.
    	virtual void setFillColor(double r, double g, double b)=0;
    	
    	// Color used for outline of shapes.
    	virtual void setStrokeColor(double r, double g, double b)=0;
 
 		//Width of lines
 		virtual void setStrokeWidth(double w)=0;
 		
 		//dashed stroke
 		virtual void setDashed(int dashed)=0;
 		  		
    		
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
			const std::vector<point> &vertices,
			const std::vector<triangle> &triangles)=0;
			
        virtual void drawTriangles(
            const std::vector<triangle> &triangles)=0;
            
        virtual void drawTriangles(
            const std::vector<triangle> *triangles)=0;

        virtual void drawQuads(
            const std::vector<boundingBox> *quads)=0;
                
        virtual void drawGrid()=0;

       
        virtual double getPageWidth()  = 0;
        virtual double getPageHeight() = 0;
        
        virtual int saveToFile(std::string filename) = 0;        
        virtual ~Canvas(){};
    
    protected:
    
    	double lx, ly, xratio, yratio, margin;
    	
    	point windowMin, windowMax; 
    	
    	std::string filename;
		
		enum DRAW_ACTION {FILL=1, STROKE=2};

		int drawAction=STROKE;
    	
    	int yAxisFlipped = 0;

		void mapToPage(double x, double y, double &px, double &py);
		
		void mapDimensionsToPage(double width, double height, double &pwidth, double &pheight);
		
		void mapRadiusToPage(double dim, double &pdim);
		
		int hasCorrectExtension(const char * s1, const char * extension);
		
		virtual void copyNameWithExtension(std::string fname, std::string extension);
				
		
	private:

		


};

#endif
