#include "ListReader.h"
#include <assert.h>
#include <stdio.h>
#include "triangle.h"
#include "point.h"


// TIPPList<point>	ListReader::readVerticesIntoList(){
// 
// 	FILE * vfile = fopen(vertexFileName, "rb");
// 	
// 	double * buffer = malloc( 2 * sizeof(double)); 
// 	TIPPList list<point>;
// 	unsigned long long int index=0; 
// 	int numread;
// 	
// 	while ( 2 == (numread = fread(buffer, sizeof(double), 2, vfile)) ){
// 	
// 		point p( buffer[0], buffer[1], index);
// 				
// 		list.insertFront(p);
// 		index++;
// 	}
// 	
// 	assert(numread == 0); // should have hit eof after reading a complete point.
// 	
// 	free(buffer);
// 	
// 	return list;
// }


void ListReader::readPoints(){ // array

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



void ListReader::readTriangles(){

	FILE * tfile = fopen(triangleFileName.c_str(), "rb");
	unsigned long long int * buffer = (unsigned long long int *) malloc( 3 * sizeof(unsigned long long int)); 
	unsigned long long int index=0; 
	int numread;
	
	if(NULL == this->points){
		std::cout << "ListReader::readTriangles(): reading points first." << std::endl;
		this->readPoints();
	}
	
	while ( 3 == (numread = fread(buffer, sizeof(unsigned long long int), 3, tfile)) ){
	
		triangle t( points[buffer[0]], points[buffer[1]], points[buffer[2]]);
		
		this->triangles.insertFront(t);
	}
	
	assert(numread == 0); // should be no "spare change"

}
