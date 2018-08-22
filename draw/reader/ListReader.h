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
        ListReader(std::string tFileName="", std::string vFileName ="" ){ 
        
            vertexFileName = vFileName;
            triangleFileName = tFileName;
        }

        void readPoints();
        void readTriangles();
        
        

        TIPPList<triangle> getTriangleList();
        point * getPointArray();
        triangle * getTriangleArray(int &numElements); 
        
        virtual ~ListReader(){
        
        }  
        
        
    protected:
        
        // Read triangles that were written as triplets of indices, along with a separate vertex file.
        void readTrianglesWithSeparatePointsFile();
        
        // read triangles that were written out completely using fwrite(), meaning they already have points.
        // This method may not be portable, due to differences in padding, etc. 
        void readTrianglesWithFread();

        // read "flattened" triangles consisting of coordinates for each vertex of each triangle.
        void readFlattenedTriangles();

    
        std::string vertexFileName;
        std::string triangleFileName;
        
        point * points = NULL;
        TIPPList<triangle>  triangles;
        triangle * trianglesArray = NULL; 
        int numTriangles=0; //length of  trianglesArray   
};

#endif
