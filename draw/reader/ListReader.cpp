#include "ListReader.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "triangle.h"
#include "point.h"
#include "TIPPList.hpp"


// TIPPList<point>  ListReader::readVerticesIntoList(){
// 
//  FILE * vfile = fopen(vertexFileName, "rb");
//  
//  double * buffer = malloc( 2 * sizeof(double)); 
//  TIPPList list<point>;
//  unsigned long long int index=0; 
//  int numread;
//  
//  while ( 2 == (numread = fread(buffer, sizeof(double), 2, vfile)) ){
//  
//      point p( buffer[0], buffer[1], index);
//              
//      list.insertFront(p);
//      index++;
//  }
//  
//  assert(numread == 0); // should have hit eof after reading a complete point.
//  
//  free(buffer);
//  
//  return list;
// }


void ListReader::readPoints(){ // array

    if (vertexFileName == ""){
    
        std::cerr << "ListReader::readPoints(): no vertex file given."  << std::endl;
        exit(1);
    }
    
    FILE * vfile = fopen(vertexFileName.c_str(), "rb");
    
    fseek(vfile, 0, SEEK_END); // seek to end of file
    int nPoints = ftell(vfile)/(sizeof(point)); // get current file pointer
    fseek(vfile, 0, SEEK_SET); // seek back to beginning of file

    std::cout << "ListReader::readPoints(): reading " << nPoints << " from the file."  << std::endl;
        
    this->points = (point *) malloc( nPoints * sizeof(point)); 
    
    int numread = fread(this->points, sizeof(point), nPoints, vfile);
    assert(numread == nPoints);
    fclose(vfile);  
}


// read triangles that were written out completely using fwrite(), meaning they already have points.
void ListReader::readTriangles(){

    FILE * tfile = fopen(triangleFileName.c_str(), "rb");
    unsigned long long int * buffer = (unsigned long long int *) malloc( 3 * sizeof(unsigned long long int)); 
    unsigned long long int index=0; 
    int numread;
    
    if(vertexFileName == ""){
        this->readTrianglesWithSingleFile();
    } else {
        this->readTrianglesWithSeparatePointsFile();
    }
}





// Read triangles that were written as triplets of indices, along with a separate vertex file.
void ListReader::readTrianglesWithSeparatePointsFile(){

    FILE * tfile = fopen(triangleFileName.c_str(), "rb");
    unsigned long long int * buffer = (unsigned long long int *) malloc( 3 * sizeof(unsigned long long int)); 
    unsigned long long int index=0; 
    int numread;
    
    if( NULL == buffer){
        std::cerr << "ListReader::readTrianglesWithSeparatePointsFile(): malloc failed" << std::endl;
        exit(1);
    }

    if(NULL == this->points){
        std::cout << "ListReader::readTrianglesWithSeparatePointsFile(): calling readPoints()" << std::endl;
        this->readPoints();
    }
    
    while ( 3 == (numread = fread(buffer, sizeof(unsigned long long int), 3, tfile)) ){
    
        triangle t( points[buffer[0]], points[buffer[1]], points[buffer[2]]);
        this->triangles.insertFront(t);
    }
    
    assert(numread == 0); // should be no "spare change"
}





// read triangles that were written out completely using fwrite(), meaning they already have points.
void ListReader::readTrianglesWithSingleFile(){

    FILE * tfile = fopen(triangleFileName.c_str(), "rb");
    
    fseek(tfile, 0, SEEK_END); // seek to end of file
    int nTriangles = ftell(tfile)/(sizeof(triangle)); // get current file pointer
    fseek(tfile, 0, SEEK_SET); // seek back to beginning of file
    
    this->trianglesArray = (triangle *) malloc( nTriangles * sizeof(triangle)); 
    
    if( NULL == this->trianglesArray){
        std::cerr << "ListReader::readTrianglesWithSingleFile(): malloc failed" << std::endl;
    }
    
    int numread = fread(this->trianglesArray, sizeof(triangle), nTriangles, tfile);
    
    if( numread != nTriangles){
        std::cerr << "ListReader::readTrianglesWithSingleFile(): fread failed" << std::endl;
    } else {
        std::cout << "ListReader::readTrianglesWithSingleFile(): Successfully read " << numread << " triangles." << std::endl;
    }
    this->numTriangles = numread;
}


point * ListReader::getPointArray(){
        
    return this->points;
}
        
TIPPList<triangle>  ListReader::getTriangleList(){

    return this->triangles;  
}

triangle * ListReader::getTriangleArray(int &numElements){

    numElements = this->numTriangles;
    return this->trianglesArray; 
}




