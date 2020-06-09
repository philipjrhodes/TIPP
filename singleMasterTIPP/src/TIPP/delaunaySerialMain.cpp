#include "domain.h"

//=============================================================================
int main (int argc, char** argv){
//=============================================================================

	if(argc<=3){// no arguments
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first two arguments are the input and output paths\n";
		std::cout<<"The third argument is the domain size\n";
		return 0;
	}

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];
	double domainSize = atof(argv[3]);
	domain *d;

	double loadTime = 0, initialTime = 0, prepareActivePartitionTime=0, delaunayTime = 0, storeTime = 0, currentTime, overAllTime = 0;

	overAllTime = currentTime = GetWallClockTime();
	d = new domain(0.0,0.0, domainSize, domainSize, inputPath, outputPath);
	d->loadInitPoints();
	loadTime += GetWallClockTime()-currentTime;

	currentTime = GetWallClockTime();
	d->initTriangulate();
	initialTime += GetWallClockTime()-currentTime;

	d->triangleTransform();

	bool delaunayStop = false;//loop until process all partitions

	int stage = 1;
	//Number of current active partitions
	unsigned int activePartNum;
	while(!delaunayStop){

		unsigned currActivePartNum;
		d->generateIntersection();
		d->generateConflictPartitions();
		activePartNum = d->generateActivePartitions();
		d->updateConflictPartitions();
		d->deliverTriangles();

		std::cout<<"*****************************STAGE: "<<stage<<"*****************************\n";
		currentTime = GetWallClockTime();
		d->prepareAndDelaunayIndependentPartitions(storeTime);
		delaunayTime += GetWallClockTime() - currentTime;
		delaunayTime -= storeTime;

		//currActivePartNum is thenumber of active partition leftover for the next loop.
		//it means that if currActivePartNum==0, 
		//then we have to process Delaunay MPI for the last shift in current stage.
		currActivePartNum = d->activePartitionNumber();
		if(d->unfinishedPartNum()<=0) delaunayStop = true;

		d->updateTriangleArrForSerial();
		stage++;
	}

	currentTime = GetWallClockTime();
	d->storeAllTriangles();
	overAllTime = GetWallClockTime() - overAllTime;

	std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
	std::cout<<"====================================================================\n";
	std::cout<<"done!!!"<<std::endl;
	std::cout<<"Serial version (one-master)\n";
	std::cout<<"Datasources: "<<outputPath<<std::endl;
	std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;
	unsigned int  pointPartNum = d->pointNumMax/(d->xPartNum*d->yPartNum);
	std::cout<<"Average number of points per partition: "<<pointPartNum<<"\n";

	prepareActivePartitionTime = overAllTime - (loadTime + initialTime + delaunayTime + storeTime);
	std::cout<<"\nload init triangle time: "<<loadTime<<"\n";
	std::cout<<"Initial time: "<<initialTime<<"\n";
	std::cout<<"Prepare active partition time: "<<prepareActivePartitionTime<<"\n";
	std::cout<<"Delaunay time: "<<delaunayTime<<"\n";
	std::cout<<"Store time: "<<storeTime<<"\n";
	std::cout<<"Total time: "<<overAllTime<<"\n";
	std::cout<<"====================================================================\n";

	//Write to result file (result.txt) in current folder
	std::ofstream resultFile;
	resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
	resultFile<<"\n\nOne-Master"<<", "<<outputPath<<", "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<", "<<loadTime<<" "<<initialTime<<" "<<prepareActivePartitionTime<<" "<<delaunayTime<<" "<<storeTime<<" "<<overAllTime<<"\n";

	resultFile.close();
	delete d;
}
