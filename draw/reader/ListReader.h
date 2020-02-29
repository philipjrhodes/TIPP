#ifndef LISTREADER_H
#define LISTREADER_H

#include <string>
#include "TIPPList.hpp"
#include "point.h"
#include "triangle.h"

class ListReader{

    public:


        // if vFileName is ommitted or set to "", then we assume the triangle file
        // contains complete points.
        ListReader(std::string tFileName="", std::string vFileName ="", std::string qFileName ="" ){ 
        
            vertexFileName = vFileName;
            triangleFileName = tFileName;
            quadFileName = qFileName;
        }

        void readPoints();
        void readTriangles();
        
        void setQuadsFileName(std::string qFileName);
        void readQuads();// read a list of boundingBoxes from a quad file (txt)
        

        TIPPList<triangle> getTriangleList();
        point * getPointArray();
        triangle * getTriangleArray(int &numElements); 
        
        std::vector<boundingBox> * getQuadList();
        
        virtual ~ListReader(){
        
        }  
        
        
    protected:
        
        // Read triangles that were written as triplets of indices, along with a separate vertex file.
        void readTrianglesWithSeparatePointsFile();
 
        // Read Points that were written as pairs of doubles.
        void readPointsAsDoubles();

        
        // read triangles that were written out completely using fwrite(), meaning they already have points.
        // This method may not be portable, due to differences in padding, etc. 
        void readTrianglesWithFread();

        // read "flattened" triangles consisting of coordinates for each vertex of each triangle.
        void readFlattenedTriangles();
        
        


    
        std::string vertexFileName;
        std::string triangleFileName;
        std::string quadFileName;
        
        
        point * points = NULL;
        TIPPList<triangle>  triangles;
        triangle * trianglesArray = NULL; 
        int numTriangles=0; //length of  trianglesArray   
        
        std::vector<boundingBox> * quads=NULL;
};

#endif
