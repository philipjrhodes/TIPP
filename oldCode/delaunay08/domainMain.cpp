//g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp domain.cpp domainMain.cpp linkList.cpp drawMesh.cpp -o domain -lgrap

#include <iostream>
#include <fstream>
#include "domain.h"
//#include "common.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	if(argc==1)// no arguments
		std::cout<<"You need to provide two arguments: path and current number of core availables\n";
	else{
		std::string path = argv[1];
		int coreNum = atoi(argv[2]);
		domain *d = new domain(0.0,0.0,1.0,1.0, path);

		double totalTime = GetWallClockTime();
		double MPITime, totalMPITime = 0;

		double currentTime = totalTime;
		d->loadInitPoints();
		std::cout<<"Time for loadInitPoints() is "<<GetWallClockTime()-currentTime<<std::endl;

		currentTime = GetWallClockTime();
		d->loadExtensionPoints();
		std::cout<<"Time for loadExtensionPoints() is "<<GetWallClockTime()-currentTime<<std::endl;

		currentTime = GetWallClockTime();
//		d->initTriangulate();
		d->initTriangulateAdvance();
		std::cout<<"Time for initTriangulate() is "<<GetWallClockTime()-currentTime<<std::endl;

//		d->drawTriangles();
		currentTime = GetWallClockTime();
		d->triangleTransform();
		std::cout<<"Time for triangleTransform() is "<<GetWallClockTime()-currentTime<<std::endl;

		currentTime = GetWallClockTime();
		while(d->unfinishedPartNum()>0){
//		for(int i=0; i<4; i++){
			d->generateIntersection();
			d->generateConflictPartitions();
//			d->printConflictPartitions();
			d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();
//			d->drawActivePartTriangles();
			
			MPITime = d->processDelaunayMPI(coreNum);
			totalMPITime = totalMPITime + MPITime;

			d->updateTriangleArr();
//			d->printTriangleArray();
		}
		std::cout<<"Time for generateIntersection(),generateConflictPartitions(),generateActivePartitions(), updateConflictPartitions(), deliverTriangles(); is "<<GetWallClockTime()-currentTime<<std::endl;

	d->storeAllTriangles();
std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";

	//draw triangles left in memory after storing some finished triangles
//	d->drawTriangleArr1();

	//draw store triangles in (fullPointPart.ver and triangleIds.tri)		
	d->drawTriangleArr();

	delete d;

	std::cout<<"done!!!"<<std::endl;
	std::cout<<"datasources: "<<path<<std::endl;
	std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;
	totalTime = GetWallClockTime() - totalTime;
	std::cout<<"MPI time: "<<totalMPITime<<"\n";
	std::cout<<"Management time: "<<totalTime - totalMPITime<<"\n";
	std::cout<<"Total time: "<<totalTime<<"\n";


	//Write to result file (result.txt) in current folder
	std::ofstream resultFile;
	resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
	resultFile<<"\n\ndatasources: "<<path<<"\n";
	resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<"\n";
	resultFile<<"MPI time: "<<totalMPITime<<"\n";
	resultFile<<"Management time: "<<totalTime - totalMPITime<<"\n";
	resultFile<<"Total time: "<<totalTime<<"\n";
	resultFile.close();

	}
}
