/* parallel MPI read with arbitrary number of processes*/
#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <string>

struct point{
	double x;
	double y;
	unsigned long int Id;
};

void createBinFile(point *data, int size, std::string fileName){
	FILE *f = fopen(fileName.c_str(), "wb");
	fwrite(data, size, sizeof(point), f);
	fclose(f);
}

int main(int argc, char *argv[])
{
	int myrank, numprocs, bufsize, *buf, count;
	MPI_File thefile;
	MPI_Status status;
	MPI_Offset filesize;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

	if (myrank==0){
		point data1[] = {{0.1,0.2, 1}, {0.2, 0.3, 2}};
		point data2[] = {{0.4,0.5, 3}, {0.6, 0.7, 4}};
		point data3[] = {{0.8,0.9, 5}, {1.0, 1.1, 6}};
		point data4[] = {{1.2,1.3, 7}, {1.4, 1.5, 8}};
		createBinFile(data1, 2, "test1");
		createBinFile(data2, 2, "test2");
		createBinFile(data3, 2, "test3");
		createBinFile(data4, 2, "test4");
	}

	std::string fileName;
	if(myrank==0) fileName = "test1";
	if(myrank==1) fileName = "test2";
	if(myrank==2) fileName = "test3";
	if(myrank==3) fileName = "test4";

//	sprintf(fileName, "test%d", myrank+1);
	FILE *f = fopen(fileName.c_str(), "rb");
	//pointCoorArr is an array of points
	point *data = new point[2];
	fread(data, 2, sizeof(point), f);
	fclose(f);
	
	MPI_Barrier(MPI_COMM_WORLD);
if(myrank==3){
	for(int i=0; i<2; i++) std::cout<<data[i].x<<" "<<data[i].y<<" "<<data[i].Id<<std::endl;
}
	delete [] data;



















/*
	if (myrank==0){
		int data1[] = {1, 2, 3, 4, 5};
		int data2[] = {6, 7, 8, 9, 10};
		int data3[] = {11, 12, 13, 14, 15};
		int data4[] = {16, 17, 18, 19, 20};
		createBinFile(data1, 5, "test1");
		createBinFile(data2, 5, "test2");
		createBinFile(data3, 5, "test3");
		createBinFile(data4, 5, "test4");
	}

	std::string fileName;
	if(myrank==0) fileName = "test1";
	if(myrank==1) fileName = "test2";
	if(myrank==2) fileName = "test3";
	if(myrank==3) fileName = "test4";

//	sprintf(fileName, "test%d", myrank+1);
	FILE *f = fopen(fileName.c_str(), "rb");
	//pointCoorArr is an array of points
	int *data = new int[5];
	fread(data, 5, sizeof(int), f);
	fclose(f);
	
	MPI_Barrier(MPI_COMM_WORLD);
if(myrank==3){
	for(int i=0; i<5; i++) std::cout<<data[i]<<" ";
	std::cout<<std::endl;
}
	delete [] data;

*/

//	int size[] = {4,1,6,11};

//	std::string fileStr = generateFileName(myrank, "../dataSources/100vertices/delaunayResults/pointPart", 16);
//	sprintf(fileStr, "pointPart0%d.ver", myrank);

/*
	std::string fileStr = "../dataSources/100vertices/delaunayResults/pointPart00";
	FILE *f = fopen(fileStr.c_str(), "rb");
	//pointCoorArr is an array of points
	point *pointCoorArr = new point[1];
	fread(pointCoorArr, 1, sizeof(point), f);
	fclose(f);
*/

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
