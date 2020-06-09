//Producer consumer version only works with shared folder. 
//It means that worker processes read data points directly from shared folder
#include "domain.h"
#include "delaunayMPI_ProducerConsumer.h"

//=============================================================================
int main (int argc, char** argv){
//=============================================================================

	if(argc<=3){// no arguments
		std::cout<<"You need to provide arguments:\n";
		std::cout<<"The first two arguments are the input and output paths\n";
		std::cout<<"The third argument is the domain size\n";
		return 0;
	}

    int my_rank, pool_size;
	int processNum;

	std::string inputPath = argv[1];
	std::string outputPath = argv[2];
	double domainSize = atof(argv[3]);
	//shareFolderrOption=1 --> worker processes read point data directly from share folder
	//shareFolderrOption=0 --> master process read point data for all worker processes and send to them
	bool shareFolderOption = atoi(argv[4]);

	domain *d;
	delaunayMPI_ProducerConsumer *pMPI_ProducerConsumer;

	pMPI_ProducerConsumer = new delaunayMPI_ProducerConsumer(domainSize, shareFolderOption, inputPath, outputPath);

	double loadInitPointTime = 0, initialTime = 0, masterTime = 0, updateTime = 0, readTime = 0, storeTime = 0, IOtime=0, workerTime=0;
	double MPITriangulationTime = 0, MPIStoreTime = 0, MPIReadTime = 0;
	double currentTime, amountTime, overAllTime;

	//ACTION - start do to parallel
    MPI_Init(&argc, &argv);

	overAllTime = MPI_Wtime();

	//Get the individual process ID.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	//pool_size has to be equaled to number of partitions, each partition has some triangles
	//Get the number of processes.
	MPI_Comm_size(MPI_COMM_WORLD, &pool_size);
	processNum = pool_size;

	if(my_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d = new domain(0.0,0.0, domainSize, domainSize, inputPath, outputPath);
		d->loadInitPoints();
		loadInitPointTime = GetWallClockTime()-currentTime;
		std::cout<<"Time for loadInitPoints() is "<<loadInitPointTime<<std::endl;

		currentTime = GetWallClockTime();
		d->initTriangulate();
		initialTime = GetWallClockTime()-currentTime;
		std::cout<<"Time for initTriangulate() is "<<initialTime<<std::endl;

		currentTime = GetWallClockTime();
		d->triangleTransform();
		amountTime = GetWallClockTime()-currentTime;
		initialTime += amountTime;
		std::cout<<"Time for triangleTransform() is "<<amountTime<<std::endl;
	}

	bool delaunayStop = false;//loop until process all partitions
	bool activePartStop = false;//loop until process all active partitions in a stage

	currentTime = GetWallClockTime();

	int stage = 1;
	unsigned int activePartNum;
	while(!delaunayStop){
		if(my_rank==MASTER_RANK){
			currentTime = GetWallClockTime();
			double amountTime1, amountTime2;
			d->generateIntersection();
			d->generateConflictPartitions();
			d->printConflictPartitions();
			activePartNum = d->generateActivePartitions();
			d->updateConflictPartitions();
			amountTime = GetWallClockTime()-currentTime;

			d->deliverTriangles();

			if(d->unfinishedPartNum()<=0) delaunayStop = true;
			activePartStop = false;
			std::cout<<"*****************************STAGE: "<<stage<<"*****************************\n";
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);
		MPI_Bcast(&delaunayStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

		int currActivePartNum;
		while(!activePartStop){
			if(my_rank==MASTER_RANK){
				double amountTime1, amountTime2;
				//collect triangles for the group of active partitions
				//reduce number of active partititons in activePartSet
				d->prepareDataForDelaunayMPI_ProducerConsumer();

				//currActivePartNum is thenumber of active partition leftover for the next loop.
				//it means that if currActivePartNum==0, 
				//then we have to process Delaunay MPI for the last shift in current stage.
				currActivePartNum = d->activePartitionNumber();
				if(currActivePartNum<=0) activePartStop = true;
				std::cout<<"Number of active partitions leftover: "<<currActivePartNum<<"\n";
			}
			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			MPI_Bcast(&activePartStop, 1, MPI_BYTE, MASTER_RANK, MPI_COMM_WORLD);

			//process MPI
			pMPI_ProducerConsumer->processMPI(my_rank, pool_size);

			if(my_rank==MASTER_RANK){
				workerTime += MPITriangulationTime;
				storeTime += MPIStoreTime;
				readTime += MPIReadTime;
			}

			//sychronize all processes
			MPI_Barrier(MPI_COMM_WORLD);

			if(my_rank==MASTER_RANK)
				d->addReturnTriangles();

			MPI_Barrier(MPI_COMM_WORLD);
		}
		if(my_rank==MASTER_RANK){
			d->updateTriangleArr();
			stage++;
		}
		//sychronize all processes
		MPI_Barrier(MPI_COMM_WORLD);
	}

	delete pMPI_ProducerConsumer;

	if(my_rank==MASTER_RANK){
		currentTime = GetWallClockTime();
		d->storeAllTriangles();
		//d->combineFiles();
		storeTime += GetWallClockTime()-currentTime;


		std::cout<<"Number of undelivered triangles left: "<<d->unDeliveredTriangleNum()<<"\n";
		std::cout<<"====================================================================\n";
		overAllTime = MPI_Wtime()-overAllTime;
		std::cout<<"done!!!"<<std::endl;
		std::cout<<"Single master version\n";
		std::cout<<"Datasources: "<<inputPath<<std::endl;
		std::cout<<"Delaunay triangulation: "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<std::endl;
		std::cout<<"Number of processes: "<<pool_size<<std::endl;
		unsigned pointPartNum = d->pointNumMax/(d->xPartNum*d->yPartNum);
		std::cout<<"#Points/sub-partition: "<<pointPartNum<<"\n";

		std::cout<<"\nLoad point time: "<<loadInitPointTime<<"\n";
		std::cout<<"Initial time: "<<initialTime<<"\n";
		std::cout<<"Read time: "<<readTime<<"\n";
		std::cout<<"Store time: "<<storeTime<<"\n";
		IOtime = loadInitPointTime + readTime + storeTime;
		masterTime = overAllTime - (workerTime + IOtime);

		std::cout<<"Master time: "<<masterTime<<"\n";
		std::cout<<"Worker time: "<<workerTime<<"\n";
		std::cout<<"I/O time: "<<IOtime<<"\n";
		std::cout<<"Total time: "<<overAllTime<<"\n";
		std::cout<<"====================================================================\n";

		//Write to result file (result.txt) in current folder
		std::ofstream resultFile;
		resultFile.open ("result.txt", std::ofstream::out | std::ofstream::app);

		resultFile<<"\n\nSingle-Master"<<", "<<inputPath<<", "<<d->xPartNum<<" x "<<d->yPartNum<<" = "<<d->xPartNum*d->yPartNum<<", #Points/sub-partition: "<<pointPartNum<<",  #processes: "<<pool_size<<",\nloadInitPointTime: "<<loadInitPointTime<<", initialTime: "<<initialTime<<", updateTime: "<<updateTime<<", readTime: "<<readTime<<", storeTime: "<<storeTime<<", masterTime: "<<masterTime<<", workerTime: "<<workerTime<<", masterTime & workerTime: "<<masterTime + workerTime<<", I/OTime: "<<IOtime<<", overAllTime: "<<overAllTime<<"\n";

		resultFile.close();
	}
	if(my_rank==MASTER_RANK) delete d;
    MPI_Finalize();
}
