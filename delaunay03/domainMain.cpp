//g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp domain.cpp domainMain.cpp linkList.cpp drawMesh.cpp -o domain -lgrap

#include <iostream>
#include "domain.h"
#include "common.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	if(argc==1)// no arguments
		std::cout<<"You need to provide two arguments: path and number of partitions\n";
	else{
		std::string path = argv[1];
		domain *d = new domain(0.0,0.0,1.0,1.0, path);

		double totalTime = GetWallClockTime();
		double MPITime = 0;
		double currMPITime;

		d->loadInitPoints();
		d->loadExtensionPoints();
		d->initTriangulate();
//		d->drawTriangles();
		d->triangleTransform();

		while(d->unfinishedPartNum()>0){
//		for(int i=0; i<1; i++){
			d->generateIntersection();
			d->generateConflictPartitions();
//			d->printConflictPartitions();
			d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();
//			d->drawActivePartTriangles();
			d->storeActiveParition();

			currMPITime = GetWallClockTime();
			d->distributeDelaunay();
			currMPITime = GetWallClockTime() - currMPITime;
			MPITime = MPITime + currMPITime;
			
			d->updateTriangleArr();
			d->collectStoreTriangleIds();
//			d->printTriangleArray();
		}

	d->storeAllTriangles();
std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";

	//draw triangles left in memory after storing some finished triangles
//	d->drawTriangleArr1();

	//draw store triangles in (fullPointPart.ver and triangleIds.tri)		
//	d->drawTriangleArr();

	delete d;


	std::cout<<"done!!!"<<std::endl;
	std::cout<<"datasources: "<<path<<std::endl;
	std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;

	totalTime = GetWallClockTime() - totalTime;
	std::cout<<"MPI time: "<<MPITime<<"\n";
	std::cout<<"Management time: "<<totalTime - MPITime<<"\n";
	std::cout<<"Total time: "<<totalTime<<"\n";

	}
}
