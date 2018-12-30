#ifndef PDFCANVAS_H
#define PDFCANVAS_H

#include <string>
#include <vector>

#include "Canvas.h"
#include "point.h"
#include "triangle.h"
#include "hpdf.h"

class PDFCanvas: public Canvas {
   public:
   
        PDFCanvas(  
            const std::string page_title="",
            int flipYAxis=0,    
            double lx=0,    double ly=0, 
            double ux=1,    double uy=1
        );
 
        // Color used for interior of shapes.
        virtual void setFillColor(double r, double g, double b);
        
        // Color used for outline of shapes.
        virtual void setStrokeColor(double r, double g, double b);

        //Width of lines. Initially 0.01
        virtual void setStrokeWidth(double w);          

        
        virtual void drawTriangle(
            double        x0,
            double        y0,
            double        x1,
            double        y1,
            double        x2,
            double        y2);
          
        virtual void drawCircle(
            double cx,
            double cy,
            double r);
        
        virtual void drawRect(
            double xmin,
            double ymin,
            double width,
            double height);

        virtual void drawTriangles(
            const std::vector<point> &vertices,
            const std::vector<triangle> &triangles);
            
        virtual void drawTriangles(
            const std::vector<triangle> &triangles);
            
        virtual void drawTriangles(
            const std::vector<triangle> *triangles);


        virtual double getPageWidth();
        
        virtual double getPageHeight();
 
        virtual int saveToFile(std::string filename);
        
        virtual ~PDFCanvas();       
        
    private:        
        void strokefill();
        
        HPDF_Doc  pdf=NULL;
        HPDF_Font font=NULL;
        HPDF_Page page=NULL;
};

#endif

