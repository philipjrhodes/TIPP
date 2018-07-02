//g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp domain.cpp domainMain.cpp linkList.cpp drawMesh.cpp -o domain -lgrap

#include <iostream>
#include "domain.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	if(argc==1)// no arguments
		std::cout<<"You need to provide two arguments: path and number of partitions\n";
	else{
		std::string path = argv[1];
		domain *d = new domain(0.0,0.0,1.0,1.0, path);

		double t = GetWallClockTime();
		d->loadInitPoints();
		d->initTriangulate();
//		d->drawTriangles();
		d->triangleTransform();
		int nLoop = 0;
		while(d->unfinishedPartNum()>0){
//		for(int i=0; i<6; i++){
			d->generateIntersection();

//std::cout<<"aaaaaaaaaaaaaaaaaa\n";
			d->generateConflictPartitions();

//			d->printConflictPartitions();
			d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();
			//d->drawActivePartTriangles();
			d->storeActiveParition();

			d->distributeDelaunay();
			d->updateTriangleArr();
			d->collectStoreTriangleIds();
//			d->printTriangleArray();
			nLoop++;
		}
	d->storeAllTriangles();
std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";

	//draw triangles left in memory after storing some finished triangles
//	d->drawTriangleArr1();

	//draw store triangles in (fullPointPart.ver and triangleIds.tri)		
//	d->drawTriangleArr();

	delete d;
	std::cout<<"Number of sending jobs to MPI: "<<nLoop<<std::endl;
	std::cout<<"Done!!!"<<std::endl;
	t = GetWallClockTime() - t;
	std::cout<<"Delaunay time: "<<t<<"\n";

	}
}
