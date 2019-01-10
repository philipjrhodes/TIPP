#include "PDFCanvas.h"
#include "hpdf.h"
#include "point.h"
#include "triangle.h"

#include <vector>
#include <string>

//using namespace std;

void error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    printf ("ERROR: error_no=0x%04X, detail_no=%d\n",
      (unsigned int) error_no, (int) detail_no);
    throw std::exception (); /* throw exception on error */
}



PDFCanvas::PDFCanvas(   const std::string page_title,   
                        int flipYAxis,
                        double lx,  double ly, 
                        double ux,  double uy): Canvas(flipYAxis){
                        

    this->pdf = HPDF_New (error_handler, NULL);
    if (!this->pdf) {
        printf ("error: cannot create PdfDoc object\n");
    }
    
    /* create default-font */
    this->font = HPDF_GetFont(this->pdf, "Helvetica", NULL);

    /* add a new page object. */
    page = HPDF_AddPage (this->pdf);
    
    HPDF_Page_SetSize(this->page, HPDF_PAGE_SIZE_LETTER, HPDF_PAGE_PORTRAIT);

    HPDF_Page_SetLineWidth (this->page, 0.01);
    
    double width  = HPDF_Page_GetWidth(this->page);
    double height = HPDF_Page_GetHeight(this->page); 

    this->setMapping(lx, ly, ux, uy, width, height);

    printf("width = %lf, height = %lf\n", width, height);
    
    HPDF_Page_Rectangle (this->page, this->margin, this->margin, width-2*(this->margin), height-2*(this->margin));
    HPDF_Page_Clip (this->page);
    HPDF_Page_EndPath(this->page); // instead of drawing the rectangle.

    /* print the title of the page (with positioning center). */
    HPDF_Page_SetFontAndSize (this->page, this->font, 12);
    float tw = HPDF_Page_TextWidth (this->page, page_title.c_str());
    HPDF_Page_BeginText (this->page);
    HPDF_Page_MoveTextPos (this->page, (HPDF_Page_GetWidth(this->page) - tw) / 2,
                HPDF_Page_GetHeight (this->page) - 50);
    HPDF_Page_ShowText (this->page, page_title.c_str());
    HPDF_Page_EndText (this->page);

    HPDF_Page_SetFontAndSize (this->page, this->font, 10);
}

// Color used for interior of shapes.
void PDFCanvas::setFillColor(double r, double g, double b){

    HPDF_Page_SetRGBFill(this->page, r, g, b);
}

// Color used for outline of shapes.
void PDFCanvas::setStrokeColor(double r, double g, double b){

    HPDF_Page_SetRGBStroke(this->page, r, g, b);
}

void PDFCanvas::strokefill(){

    switch(drawStyle){
        
        case FILL:
            HPDF_Page_Fill(this->page);
            break;
            
        case STROKE:
            HPDF_Page_Stroke(this->page);
            break;
            
        case FILL|STROKE:
            HPDF_Page_FillStroke(this->page);
            break;
            
        default:
            std::cerr << "invalid drawStyle\n" << std::endl;    
    }       
}

void PDFCanvas::setStrokeWidth(double w){

    HPDF_Page_SetLineWidth (this->page, w);
}

void PDFCanvas::drawTriangle(
    double        x0,
    double        y0,
    double        x1,
    double        y1,
    double        x2,
    double        y2) {
   
        double px, py;
   
        mapToPage(x0, y0, px, py);
        HPDF_Page_MoveTo(this->page, px, py);
    
        mapToPage(x1, y1, px, py);
        HPDF_Page_LineTo(this->page, px, py);
    
        mapToPage(x2, y2, px, py);
        HPDF_Page_LineTo(this->page, px, py);
        
        HPDF_Page_ClosePath(this->page);
        
        strokefill();
}

void PDFCanvas::drawCircle(
    double cx,
    double cy,
    double r){

        HPDF_Page_Circle(this->page, cx, cy, r);
        
        strokefill();
}

void PDFCanvas::drawRect(
    double xmin,
    double ymin,
    double width,
    double height){

        HPDF_Page_Rectangle(this->page, xmin, ymin, width, height);
        
        strokefill();
}

// Uses triangle Ids to index vertices.
void PDFCanvas::drawTriangles(const std::vector<point> &vertices, const std::vector<triangle> &triangles){

//    std::cerr << "drawTriangles()[vector<point>, vector<triangle>]: Not implemented." << std::endl;
//    return;
    int numTriangles = triangles.size();
    
    std::cerr << "drawTriangles(): numTriangles == " << numTriangles << std::endl;
    for(int i=0; i< numTriangles; i++){
                    
		drawTriangle(	vertices[triangles[i].p1.Id].x, vertices[triangles[i].p1.Id].y, 
						vertices[triangles[i].p2.Id].x, vertices[triangles[i].p2.Id].y,
						vertices[triangles[i].p3.Id].x, vertices[triangles[i].p3.Id].y
					);	
    }
}

// Assumes points within triangle objects are valid.
void PDFCanvas::drawTriangles(const std::vector<triangle> &triangles){

    int numTriangles = triangles.size();
    
    std::cerr << "drawTriangles()[flattened triangles]: numTriangles == " << numTriangles << std::endl;
    for(int i=0; i< numTriangles; i++){
                    
        drawTriangle(   triangles[i].p1.x, triangles[i].p1.y, 
                        triangles[i].p2.x, triangles[i].p2.y,
                        triangles[i].p3.x, triangles[i].p3.y
                    );  
    }
}

// Assumes points within triangle objects are valid.
void PDFCanvas::drawTriangles(const std::vector<triangle> *triangles){

    int numTriangles = triangles->size();
    
    std::cerr << "drawTriangles()[flattened triangles *]: numTriangles == " << numTriangles << std::endl;
    for(int i=0; i< numTriangles; i++){
                    
        drawTriangle(   (*triangles)[i].p1.x, (*triangles)[i].p1.y, 
                        (*triangles)[i].p2.x, (*triangles)[i].p2.y,
                        (*triangles)[i].p3.x, (*triangles)[i].p3.y
                    );  
    }
}


double PDFCanvas::getPageWidth(){

    return HPDF_Page_GetWidth(this->page);
}

double PDFCanvas::getPageHeight(){

    return HPDF_Page_GetHeight (this->page);
}



int PDFCanvas::saveToFile(std::string fname){

    copyNameWithExtension(fname, ".pdf");
    
    /* save the document to a file */
    HPDF_SaveToFile (this->pdf, filename.c_str());

    /* clean up */
    HPDF_Free (this->pdf);
    this->pdf = NULL;
    
    return 0;
}


PDFCanvas::~PDFCanvas(){

    if(this->pdf)
        HPDF_Free (this->pdf);

    
}
