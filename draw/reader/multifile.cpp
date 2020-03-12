#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include "PDFCanvas.h"
#include "TIPPList.hpp"
#include "triangle.h"
#include "point.h"
#include "edge.h"

#include "ListReader.h"

using namespace std;


void usage(){
    cout << "Usage: mfd  { file1.tri [file2.ver][file3.quad]  {DARK | LIGHT | OUTLINE} }+" << endl;
    exit(1);
}

enum DrawStyle { DARK, LIGHT, OUTLINE, RED, INVALID};

DrawStyle toStyle(string s){

    if(s.compare("DARK") == 0)
        return DrawStyle::DARK;
        
    if(s.compare("LIGHT") == 0)
        return DrawStyle::LIGHT;
        
    if(s.compare("OUTLINE") == 0)
        return DrawStyle::OUTLINE;
 
    if(s.compare("RED") == 0)
        return DrawStyle::RED;
   
    return DrawStyle::INVALID;
}

int hasExtension(const char * s1, const char * extension){

    int len = strlen(s1);
    int extLen = strlen(extension);
        
    return 0 == strncmp(s1 + len - extLen, extension, extLen); 
}

struct arginfo {
    
    string tFileName;
    string vFileName;
    string qFileName;
    
    DrawStyle style;
    
    std::vector<triangle> * triangles;
    std::vector<boundingBox> * quads;
};

vector<arginfo> * parseArgs(int argc, char * argv[]){

    if(argc < 3 )
        usage();

    vector<arginfo> * v = new vector<arginfo>();
    arginfo ai;
    // triplets  tri [ver] [quad] style 
    int i=1;
    while(i<argc){
        ai.tFileName.assign(""); 
        ai.vFileName.assign("");
        ai.qFileName.assign("");
        ai.style = DrawStyle::INVALID;
        ai.triangles = NULL;
        ai.quads = NULL;
        
        if(hasExtension(argv[i],"tri")){
            ai.tFileName = argv[i];  //std::cerr << ai.tFileName << std::endl;
            i++;
        } else if(hasExtension(argv[i],"quad")) {
            ai.qFileName = argv[i];
            i++;
        } else {
            std::cerr << argv[i] << " Expected filename ending in .tri or .quad" << std::endl;
            exit(1);
        }

        DrawStyle style = toStyle(argv[i]); // maybe didn't specify .ver file?
        if( DrawStyle::INVALID == style){
            if( hasExtension(argv[i],"ver")){
                ai.vFileName=argv[i];
                i++; 
            } else {
                std::cerr << argv[i] << " Expected filename ending in .ver or .quad or {DARK | LIGHT | OUTLINE | RED}" << std::endl;
                exit(1);
            }
        }  
        
        if( i >= argc){
        
            std::cerr << "Missing argument: Expected {DARK | LIGHT | OUTLINE | RED}" << std::endl;
            exit(1);
        }

        style = toStyle(argv[i]);
        if(DrawStyle::INVALID == style){
            std::cerr << argv[i] << " Expected {DARK | LIGHT | OUTLINE | RED}" << std::endl;
            exit(1);
        } else {
            ai.style = style;  
            i++;  
        }
        
        v->push_back(ai);
        std::cerr << "Added arginfo " << std::endl;
    }

    return v;
}

// mfd  foo.tri [bar.ver] DARK  baz.tri  LIGHT 
int main(int argc, char * argv[]){
    
    vector<arginfo> * v = parseArgs(argc, argv);
      
    Canvas *c = new PDFCanvas("multiple files", 1); //flipping y axis
    
 
    // for each set of files, read the triangles into a vector and
    // update the Canvas mapping, but don't draw the triangles. We
    // want to compute the min/max over all the triangles in all
    // the files before drawing.
    for(arginfo &i: *v){
    
        ListReader *r;
        
        if (!i.tFileName.empty()) {
        
            if(i.vFileName.empty()){
            
                r = new ListReader(i.tFileName);
            } else {
        
                r = new ListReader(i.tFileName, i.vFileName);
            }
 
            r->readTriangles();
        
            int numElements=0;    
            triangle * tarr = r->getTriangleArray(numElements);
       
            if(NULL == tarr){
        
                cerr << "triangle array is empty." << endl;
                exit(1);
            }

            i.triangles = new std::vector<triangle>(tarr, tarr + numElements);
            cout << "triangle vector has length " << i.triangles->size() << endl;
            cout << "triangle[0] " << (*(i.triangles))[0] << endl;
      
            c->updateMapping(i.triangles);
             
        } else if (!i.qFileName.empty()){
        
            r = new ListReader(i.qFileName);
            
            std::cerr << "quadfilename: "<< i.qFileName << std::endl;
        	r->setQuadsFileName(i.qFileName);
        	r->readQuads();
         	
        	i.quads=r->getQuadList();
        	c->updateMapping(i.quads);

        } else {
            usage();
        }
          
        delete(r);
    }  
    
    
    // Draw the triangles/quads read in the loop above. The canvas will use a mapping derived
    // from the min/max coordinates computed over all the triangles from all the files.
    for(arginfo i: *v){
    
        switch(i.style){
        
            case DrawStyle::DARK:
                c->enableFill();
                c->enableStroke();
                c->setStrokeWidth(0.01);
                c->setStrokeColor(0.0 , 0.0, 0.0);
                c->setFillColor(0.9, 0.9, 1.0);     
                c->setDashed(0);
                break;
            
            case DrawStyle::LIGHT:
                c->enableFill();
                c->enableStroke();
                c->setStrokeWidth(0.01);
                c->setFillColor(0.995, 0.995, 0.995);
                c->setStrokeColor(0.6, 0.6, 0.6);    // .86 is too light .76 a bit light 
                c->setDashed(0);
                break;

            case DrawStyle::OUTLINE:
                c->disableFill();
                c->enableStroke();
                c->setStrokeWidth(0.01);
                c->setStrokeColor(0 , 0, 0);
                c->setDashed(0);
                break;

            case DrawStyle::RED:
                c->disableFill();
                c->enableStroke();
                c->setStrokeWidth(0.01);
                c->setStrokeColor(1 , 0, 0);
                c->setDashed(1);
                break;
   
            default:
                std::cerr << "Found Illegal style." << std::endl;
                break;
        }
        
        c->drawTriangles(i.triangles);
 		c->drawQuads(i.quads);
 		
        if (i.triangles)
            delete i.triangles; // shallow?
        
        if (i.quads)
        	delete i.quads;
     }
    
    
    c->saveToFile("reader.pdf");
    
	delete c;
    std::cerr << "main Done." << std::endl;

    return 0;
     
}



