//g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp domain.cpp domainMain.cpp linkList.cpp drawMesh.cpp -o domain -lgraph

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
		//if domainSize=1, then the domain is square between 0,0 - 1,1
		//if domainSize=3, then the domain is square between 0,0 - 3,3
		double domainSize = atoi(argv[2]);
		int coreNum = atoi(argv[3]);
		domain *d = new domain(0.0,0.0,domainSize,domainSize, path);

		double totalTime = GetWallClockTime();
		double MPITime=0, totalMPITime=0, masterTime = 0;

		double currentTime = GetWallClockTime();
		d->loadInitPoints();
		currentTime = GetWallClockTime()-currentTime;
		std::cout<<"Time for loadInitPoints() is "<<currentTime<<std::endl;		
		masterTime += currentTime;

		currentTime = GetWallClockTime();
		d->initTriangulate();
		currentTime = GetWallClockTime()-currentTime;
		std::cout<<"Time for initTriangulate() is "<<currentTime<<std::endl;
		masterTime += currentTime;
//		d->drawTriangles();

		currentTime = GetWallClockTime();
		d->triangleTransform();
		currentTime = GetWallClockTime()-currentTime;
		std::cout<<"Time for triangleTransform() is "<<currentTime<<std::endl;
		masterTime += currentTime;

		while(d->unfinishedPartNum()>0){
//		for(int i=0; i<1; i++){
			currentTime = GetWallClockTime();

			d->generateIntersection();
			d->generateConflictPartitions();
//			d->printConflictPartitions();
			d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();
//			d->drawActivePartTriangles();
			currentTime = GetWallClockTime() - currentTime;
			masterTime += currentTime;

			currentTime = d->processDelaunayMPI(coreNum);
			masterTime += currentTime;

			//draw returned triangles before they are further processed
			//You have to temporary comment away collectStoreTriangleIds() in processDelaunayMPI(coreNum) from domain.cpp;
			//comment out the "		while(d->unfinishedPartNum()>0){", use for(int i=0; i<1; i++){...
			//commment out d->updateTriangleArr();
			//loop only for the first stage
//			d->drawReturnAndStoreTriangles();
			currentTime = GetWallClockTime();
			d->updateTriangleArr();
			currentTime = GetWallClockTime() - currentTime;
			masterTime += currentTime;
//			d->printTriangleArray();
		}
		std::cout<<"Time for generateIntersection(),generateConflictPartitions(),generateActivePartitions(), updateConflictPartitions(), deliverTriangles(); is "<<GetWallClockTime()-currentTime<<std::endl;
		currentTime = GetWallClockTime();
		d->storeAllTriangles();
		currentTime = GetWallClockTime() - currentTime;
		masterTime += currentTime;


//		d->drawLeftOverTriangleArr();
std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";

		//draw triangles (not include grid points and 4 corners)
//		d->drawTriangleArr1();

		//draw store triangles in (fullPointPart.ver and triangleIds.tri)
//		d->drawTriangleArr();
//		d->drawBoundaryTriangleArr(18);

		delete d;

		std::cout<<"done!!!"<<std::endl;
		std::cout<<"datasources: "<<path<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;
		totalTime = GetWallClockTime() - totalTime;
		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"MPI time: "<<totalTime - masterTime<<"\n";
		std::cout<<"Total time: "<<totalTime<<"\n";


/*		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
		resultFile<<"\n\ndatasources: "<<path<<"\n";
		resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<"\n";
		resultFile<<"MPI time: "<<totalMPITime<<"\n";
		resultFile<<"Master time: "<<masterTime<<"\n";
		resultFile<<"Total time: "<<totalTime<<"\n";
		resultFile.close();

		//Clean up data after delaunay has been done.
		path = path + "delaunayResults/";
		std::string delCommand = "rm " + path + "initPoints.ver";
		system(delCommand.c_str());
		delCommand = "rm " + path + "pointCoorPart.ver";
		system(delCommand.c_str());
		delCommand = "rm " + path + "pointIdPart.ver";
		system(delCommand.c_str());
		delCommand = "rm " + path + "pointPartInfo.xfdl";
		system(delCommand.c_str());
*/
	}
}
