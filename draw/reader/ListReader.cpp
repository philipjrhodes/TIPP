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
    
    if(!vfile){
    
        std::cerr << "Couldn't open file: " << vertexFileName << std::endl;
        exit(1);
    }

    
    fseek(vfile, 0, SEEK_END); // seek to end of file
    int nPoints = ftell(vfile)/(sizeof(point)); // get current file pointer
    fseek(vfile, 0, SEEK_SET); // seek back to beginning of file

    std::cout << "ListReader::readPoints(): reading " << nPoints << " points from the file."  << std::endl;
        
    this->points = (point *) malloc( nPoints * sizeof(point)); 
    
    if( NULL == this->points){
        std::cerr << "ListReader::readPoints(): malloc failed." << std::endl;
        exit(1);
    }

    
    int numread = fread(this->points, sizeof(point), nPoints, vfile);
    assert(numread == nPoints);
    fclose(vfile);  
}

void ListReader::readPointsAsDoubles(){ // array

    if (vertexFileName == ""){
    
        std::cerr << "ListReader::readPointsAsDoubles(): no vertex file given."  << std::endl;
        exit(1);
    }
    
    FILE * vfile = fopen(vertexFileName.c_str(), "rb");
    
    if(!vfile){
    
        std::cerr << "Couldn't open file: " << vertexFileName << std::endl;
        exit(1);
    }

    
    
    
    fseek(vfile, 0, SEEK_END); // seek to end of file
    int nPoints = ftell(vfile)/(2 * sizeof(double)); // get current file pointer
    fseek(vfile, 0, SEEK_SET); // seek back to beginning of file

    std::cout << "ListReader::readPointsAsDoubles(): reading " << nPoints << " points from the file."  << std::endl;
        
    this->points = (point *) malloc( nPoints * sizeof(point)); 
    
    if( NULL == this->points){
        std::cerr << "ListReader::readPointsAsDoubles(): this->points malloc failed." << std::endl;
        exit(1);
    }
    
    double * buffer = (double *) malloc( nPoints * 2 * sizeof(double)); 
    
    if( NULL == buffer){
        std::cerr << "ListReader::readPointsAsDoubles(): buffer malloc failed." << std::endl;
        exit(1);
    }
    
    int numread = fread(buffer, 2 * sizeof(double), nPoints, vfile);
    assert(numread == nPoints);
    
    for(int i=0; i<nPoints; i++){
    
        this->points[i].x = buffer[2 * i + 0];
        this->points[i].y = buffer[2 * i + 1];
    }
    
    free(buffer);
    buffer = NULL;
    fclose(vfile);  
}



// read triangles that were written out completely using fwrite(), meaning they already have points.
void ListReader::readTriangles(){
    
    if(vertexFileName == ""){
        this->readFlattenedTriangles();
    } else {
        this->readTrianglesWithSeparatePointsFile();
    }
}


void ListReader::setQuadsFileName(std::string qFileName){

	quadFileName = qFileName;
}

void ListReader::readQuads(){

	FILE * qFile = fopen(quadFileName.c_str(), "r");
	
	if( qFile == NULL){
	
		std::cerr << "could not open quadfile: "<< quadFileName << std::endl;
		exit(1);
	}
	
	double lx,ux, ly, uy;
	int numQuads =0;
	
	quads = new std::vector<boundingBox>();
	boundingBox box;
	
	while ( 4 == fscanf(qFile, "%lf %lf %lf %lf", &lx, &ly, &ux, &uy)){
	
	
		box.setBox(lx, ly, ux, uy);
		quads->push_back(box);

		printf("Quad %d: %lf %lf %lf %lf\n",numQuads, lx, ly, ux, uy);

		numQuads++;
		
	}

	fclose(qFile);

}


std::vector<boundingBox> * ListReader::getQuadList(){

	return quads;
}


// Read triangles that were written as triplets of indices, along with a separate vertex file.
void ListReader::readTrianglesWithSeparatePointsFile(){

    FILE * tfile = fopen(triangleFileName.c_str(), "rb");
    
    if(!tfile){
    
        std::cerr << "Couldn't open file: " << triangleFileName << std::endl;
        exit(1);
    }
    
    // Deduce number of triangles represented in the file.
    fseek(tfile, 0, SEEK_END); // seek to end of file
    int nTriangles = ftell(tfile) / (3 * sizeof(unsigned long long int) ); // get current file pointer
    fseek(tfile, 0, SEEK_SET); // seek back to beginning of file
    
    

    if(NULL == this->points){
        std::cout << "ListReader::readTrianglesWithSeparatePointsFile(): calling readPointsAsDoubles()" << std::endl;
        this->readPointsAsDoubles();
        
    }
    
//     while ( 3 == (numread = fread(buffer, sizeof(unsigned long long int), 3, tfile)) ){
//     
//         triangle t( points[buffer[0]], points[buffer[1]], points[buffer[2]]);
//         this->triangles.insertFront(t);
//     }
    
    unsigned long long int  * vertexIndices = (unsigned long long int *) malloc( nTriangles * 3 * sizeof(unsigned long long int));
    
    if( NULL == vertexIndices){
        std::cerr << "ListReader::readTrianglesWithSeparatePointsFile(): malloc failed" << std::endl;
        exit(1);
    }
    
    int numread = fread(vertexIndices, 3 * sizeof(unsigned long long int) , nTriangles, tfile);

    if( numread != nTriangles){
        std::cerr << "ListReader::readTrianglesWithSeparatePointsFile(): fread failed" << std::endl;
    } else {
        std::cout << "ListReader::readTrianglesWithSeparatePointsFile(): Successfully read " << numread << " triangles." << std::endl;
    }
    this->numTriangles = numread;
        
 
	this->trianglesArray = (triangle *) malloc( this->numTriangles * sizeof(triangle)); 
    
    if( NULL == this->trianglesArray){
        std::cerr << "ListReader::readTrianglesWithSeparatePointsFile(): malloc failed" << std::endl;
    }
       
    
    triangle t;
    //Now that we have points, let's give them to the triangles.
    for(int i=0; i<nTriangles; i++){
        //std::cerr << vertexIndices[3 * i + 0] << " " <<  vertexIndices[3 * i + 1]  << " " << vertexIndices[3 * i + 2] << std::endl;
    
        t.p1 = points[ vertexIndices[3 * i + 0]];  
        t.p2 = points[ vertexIndices[3 * i + 1]];
        t.p3 = points[ vertexIndices[3 * i + 2]];
         
        this->trianglesArray[i] = t;
    }

    
   
    std::cerr << "ListReader::readTrianglesWithSeparatePointsFile(): done." << std::endl;
}





// read triangles that were written out completely using fwrite(), meaning they already have points.
// This method may not be portable, due to differences in padding, etc. 
void ListReader::readTrianglesWithFread(){

    FILE * tfile = fopen(triangleFileName.c_str(), "rb");

    if(!tfile){
    
        std::cerr << "Couldn't open file: " << triangleFileName << std::endl;
        exit(1);
    }
    
    
    fseek(tfile, 0, SEEK_END); // seek to end of file
    int nTriangles = ftell(tfile)/(sizeof(triangle)); // get current file pointer
    fseek(tfile, 0, SEEK_SET); // seek back to beginning of file
    
    this->trianglesArray = (triangle *) malloc( nTriangles * sizeof(triangle)); 
    
    if( NULL == this->trianglesArray){
        std::cerr << "ListReader::readTrianglesWithFread(): malloc failed" << std::endl;
    }
    
    int numread = fread(this->trianglesArray, sizeof(triangle), nTriangles, tfile);
    
    if( numread != nTriangles){
        std::cerr << "ListReader::readTrianglesWithFread(): fread failed" << std::endl;
    } else {
        std::cout << "ListReader::readTrianglesWithFread(): Successfully read " << numread << " triangles." << std::endl;
    }
    this->numTriangles = numread;
}


// read "flattened" triangles consisting of coordinates for each vertex of each triangle.
void ListReader::readFlattenedTriangles(){

    FILE * tfile = fopen(triangleFileName.c_str(), "rb");
    
    if(!tfile){
    
        std::cerr << "Couldn't open file: " << triangleFileName << std::endl;
        exit(1);
    }

    
    const int triangleDoubles = 3 * 2;
    const int triangleBytes = triangleDoubles * sizeof(double);
    
    // Deduce number of triangles represented in the file.
    fseek(tfile, 0, SEEK_END); // seek to end of file
    int nTriangles = ftell(tfile) / triangleBytes; // get current file pointer
    fseek(tfile, 0, SEEK_SET); // seek back to beginning of file
 
    this->trianglesArray = (triangle *) malloc( nTriangles * sizeof(triangle));
    if( NULL == this->trianglesArray){
        std::cerr << "ListReader::readFlattenedTriangles(): trianglesArray malloc failed" << std::endl;
    }

 
    double * coords = (double *) malloc( nTriangles * triangleBytes );  
    if( NULL == coords){
        std::cerr << "ListReader::readFlattenedTriangles(): coords malloc failed" << std::endl;
    }
    
    
    int numread = fread(coords, triangleBytes, nTriangles, tfile);
    
    if( numread != nTriangles){
        std::cerr << "ListReader::readFlattenedTriangles(): fread failed" << std::endl;
    } else {
        std::cout << "ListReader::readFlattenedTriangles(): Successfully read " << numread << " triangles." << std::endl;
    }
    this->numTriangles = numread;
    
    triangle t;
    //Now that we have coords for each vertex, let's give them to the triangles.
    for(int i=0; i<nTriangles; i++){
    
        t.p1.x = coords[i * 6 + 0];
        t.p1.y = coords[i * 6 + 1];
        
        t.p2.x = coords[i * 6 + 2];
        t.p2.y = coords[i * 6 + 3];

        t.p3.x = coords[i * 6 + 4];
        t.p3.y = coords[i * 6 + 5];
        
        this->trianglesArray[i] = t;
    }
    
    free(coords);
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




