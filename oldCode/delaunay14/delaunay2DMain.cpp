#include "domain.h"
#include "delaunayMPI.h"

//=============================================================================
int main (int argc, char** argv){
//=============================================================================

	if(argc<=2){// no arguments
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first argument is the path to the dataset\n";
		std::cout<<"The second argument is the domain size\n";
		return 0;
	}
    int my_rank, pool_size;
	double domainSize;
	std::string path;
	int coreNum;

	path = argv[1];
	domainSize = atof(argv[2]);

	domain *d;
	delaunayMPI *pMPI = new delaunayMPI(domainSize, path);

	double masterTime = 0;
	double amountTime = 0;
	double updateTime = 0;
	double storeTime = 0;
	double currentTime = GetWallClockTime();	
	double overAllTime;

	//ACTION - start do to parallel
	//Initialize MPI.
    MPI_Init(&argc, &argv);

	overAllTime = MPI_Wtime();

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	//pool_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
	MPI_Comm_size(MPI_COMM_WORLD, &pool_size);
	coreNum = pool_size;

	if(my_rank==MASTER_RANK){	
		d = new domain(0.0,0.0, domainSize, domainSize, path);
		d->loadInitPoints();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		std::cout<<"Time for loadInitPoints() is "<<amountTime<<std::endl;

		currentTime = GetWallClockTime();
		d->initTriangulate();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		std::cout<<"Time for initTriangulate() is "<<amountTime<<std::endl;

		currentTime = GetWallClockTime();
		d->triangleTransform();
		amountTime = GetWallClockTime()-currentTime;
		masterTime += amountTime;
		std::cout<<"Time for triangleTransform() is "<<amountTime<<std::endl;
	}

	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage

	currentTime = GetWallClockTime();

	int stage = 1;
	//Number of current active partitions
	unsigned int activePartNum;
	while(!delaunayStop){
		if(my_rank==MASTER_RANK){
			d->generateIntersection();
			d->generateConflictPartitions();
//			d->printConflictPartitions();
			activePartNum = d->generateActivePartitions();
			d->updateConflictPartitions();
			d->deliverTriangles();
			if(d->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
			std::cout<<"----------------stage: "<<stage<<"-------------------\n";

			amountTime = GetWallClockTime()-currentTime;
			masterTime += amountTime;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

		int currActivePartNum;
		while(!activePartStop){
			if(my_rank==MASTER_RANK){
				currentTime = GetWallClockTime();
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				d->prepareDataForDelaunayMPI(coreNum);

				//currActivePartNum is thenumber of active partition leftover for the next loop.
				//it means that if currActivePartNum==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				currActivePartNum = d->activePartitionNumber();
				if(currActivePartNum<=0) activePartStop = true;
				std::cout<<"Number of active partitions leftover: "<<currActivePartNum<<"\n";
				amountTime = GetWallClockTime()-currentTime;
				masterTime += amountTime;
			}
			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

			//process MPI
			pMPI->processMPI(my_rank, pool_size);

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			if(my_rank==MASTER_RANK){
				currentTime = GetWallClockTime();
				d->collectStoreTriangleIds();
				amountTime = GetWallClockTime()-currentTime;
				storeTime += amountTime;

				currentTime = GetWallClockTime();
				d->addReturnTriangles();
				amountTime = GetWallClockTime()-currentTime;
				updateTime += amountTime;
			}

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);
		}
		if(my_rank==MASTER_RANK){
			currentTime = GetWallClockTime();
			d->updateTriangleArr();
			amountTime = GetWallClockTime()-currentTime;
			updateTime += amountTime;
			stage++;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);
	}
	delete pMPI;

	if(my_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d->storeAllTriangles();

		amountTime = GetWallClockTime()-currentTime;
		storeTime += amountTime;

		std::cout<<"number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		overAllTime = MPI_Wtime()-overAllTime;
		std::cout<<"done!!!"<<std::endl;
		std::cout<<"datasources: "<<path<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;
		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"Update time: "<<updateTime<<"\n";
		std::cout<<"Store time: "<<storeTime<<"\n";
		std::cout<<"MPI time: "<<overAllTime - (masterTime + updateTime + storeTime)<<"\n";
		std::cout<<"Total time: "<<overAllTime<<"\n";

		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);
		resultFile<<"\n\ndatasources: "<<path<<"\n";
		resultFile<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<"\n";
		resultFile<<"Master time: "<<masterTime<<"\n";
		resultFile<<"Update time: "<<updateTime<<"\n";
		resultFile<<"Store time: "<<storeTime<<"\n";
		resultFile<<"MPI time: "<<overAllTime - (masterTime + updateTime + storeTime)<<"\n";
		resultFile<<"Total time: "<<overAllTime<<"\n";
		resultFile.close();
	}
	if(my_rank==MASTER_RANK) delete d;
    MPI_Finalize();
}
