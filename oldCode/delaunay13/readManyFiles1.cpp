/* parallel MPI read with arbitrary number of processes*/
#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "point.h"
#include "common.h"

int main(int argc, char *argv[])
{
	int myrank, numprocs, bufsize, *buf, count;
	MPI_File thefile;
	MPI_Status status;
	MPI_Offset filesize;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	int size[] = {4,1,6,11};

	std::string fileStr = generateFileName(myrank, "../dataSources/100vertices/delaunayResults/pointPart", 16);
	FILE *f = fopen(fileStr.c_str(), "rb");
	//pointCoorArr is an array of points
	point *pointCoorArr = new point[size[myrank]];
	fread(pointCoorArr, size[myrank], sizeof(point), f);
	fclose(f);
	
	if(myrank==0)
	for(int i=0; i<size[myrank]; i++)
			std::cout<<pointCoorArr[i].getX()<<" "<<pointCoorArr[i].getY()<<" "<<pointCoorArr[i].getId()<<std::endl;
	delete [] pointCoorArr;

/*	std::string fileStr = "../dataSources/100vertices/delaunayResults/pointPart00";
	point *pointCoorArr = new point[1];
	MPI_File myfile;
	MPI_File_open(MPI_COMM_WORLD, fileStr.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &myfile);
	MPI_File_seek(myfile, myrank*sizeof(point), MPI_SEEK_SET);
	MPI_File_read(myfile, pointCoorArr, 1, sizeof(point), MPI_STATUS_IGNORE);
	MPI_File_close(&myfile);
*?
/*	if(myrank==0){
	std::cout<<myrank<<" "<<fileStr<<"\n";
	for(int i=0; i<size[myrank]; i++)
			std::cout<<pointCoorArr[i].getX()<<" "<<pointCoorArr[i].getY()<<" "<<pointCoorArr[i].getId()<<std::endl;
	}
*/
//	std::cout<<pointCoorArr[0].getX()<<" "<<pointCoorArr[0].getY()<<" "<<pointCoorArr[0].getId()<<std::endl;

//	delete [] pointCoorArr;
/*	MPI_File_open(MPI_COMM_WORLD, "testfile", MPI_MODE_RDONLY, MPI_INFO_NULL, &thefile);

	MPI_File_get_size(thefile, &filesize); 
	filesize = filesize / sizeof(int);

	
	bufsize = filesize / (numprocs>10?10:numprocs);
if(myrank>=10) bufsize=0;
	
	buf = (int *) malloc (bufsize * sizeof(int));

	MPI_File_set_view(thefile, myrank * bufsize * sizeof(int),
	MPI_INT, MPI_INT, "native", MPI_INFO_NULL);

	MPI_File_read(thefile, buf, bufsize, MPI_INT, &status);

	MPI_Get_count(&status, MPI_INT, &count);

	printf("process %d read %d ints\n", myrank, count);

//	if(myrank==10){
//		int i;
//		for(i=0; i<bufsize; i++) printf("%d ", buf[i]);
//		printf("%d\n", count);
//	}

	MPI_File_close(&thefile);
*/
	MPI_Finalize();


	return 0;
}
