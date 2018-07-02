nclude <iostream>
#include <fstream>
#include "domain.h"

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv){

	if(argc==1)// no arguments
		std::cout<<"You need to provide two arguments: path and number of partitions\n";
	else{
		std::string path = argv[1];
		domain *d = new domain(0.0,0.0,1.0,1.0, path);

		double totalTime = GetWallClockTime();
		double delaunayTime = 0;
		double currDelaunayTime;

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

			currDelaunayTime = GetWallClockTime();
			d->distributeDelaunay();
			currDelaunayTime = GetWallClockTime() - currDelaunayTime;
			delaunayTime = delaunayTime + currDelaunayTime;

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
	std::cout<<"Delaunay time: "<<delaunayTime<<"\n";
	std::cout<<"Management time: "<<totalTime - delaunayTime<<"\n";
	std::cout<<"Total time: "<<totalTime<<"\n";

	//Write to result file (result.txt) in current folder
	std::ofstream resultFile;
	resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
	resultFile<<"\n\ndatasources: "<<path<<"\n";
	resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<"\n";
	resultFile<<"Delaunay time: "<<delaunayTime<<"\n";
	resultFile<<"Management time: "<<totalTime - delaunayTime<<"\n";
	resultFile<<"Total time: "<<totalTime<<"\n";
	resultFile.close();

	}
}
