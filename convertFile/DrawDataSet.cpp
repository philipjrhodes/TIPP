//g++ -std=c++11 DrawDataSet.cpp -o DrawDataSet -lgraph
#include <graphics.h>
#include <iostream>
#include <fstream>
#include <string>

int scale = 350;
int originalX = 100;
int originalY = 50;

struct Triangle{
	float coordinate[8];
};

struct intTriangle{
	int coordinate[8];
};

Triangle *triangleList;
int triangleListSize;

void loadDataSet(std::string triangleFile, std::string vertexFile){
	std::ifstream instreamTriangle(triangleFile.c_str());
	std::ifstream instreamVertex(vertexFile.c_str());
	std::string strItem;

	//read vertex file
	instreamVertex >> strItem;
	int recordSize1 = atoi(strItem.c_str());

	instreamVertex >> strItem;
	int recordCount1 = atoi(strItem.c_str());

	float *coordinateList = new float[recordCount1*recordSize1];
	for(int i=0; i<recordCount1*recordSize1; i++){
		instreamVertex >> strItem;
		coordinateList[i] = std::stod(strItem.c_str());
	}

for(int i=0; i<recordCount1*recordSize1; i++){
	if(i%2==0) std::cout<<"\n";
	std::cout<<coordinateList[i]<<" ";
}
std::cout<<"------------------------------\n";


	//read triangle file
	int recordSize2 = 3;

	instreamTriangle >> strItem;
	int recordCount2 = atoi(strItem.c_str());

	unsigned int *triangleIdList = new unsigned int[recordCount2*recordSize2];



	//load content from text file to array data
	unsigned int index = 0;
	
	for(unsigned int i=0; i<recordCount2*(recordSize2+1); i++){
		if(i%4==0)	instreamTriangle >> strItem;
		else{
			instreamTriangle >> strItem;
			triangleIdList[index] = atoi(strItem.c_str());
			index++;
		}
	}




//	for(int i=0; i<recordCount2*recordSize2; i++){
//		instreamTriangle >> strItem;
//		triangleIdList[i] = atoi(strItem.c_str());
//	}

for(int i=0; i<recordCount2*recordSize2; i++){
	if(i%3==0) std::cout<<"\n";
	std::cout<<triangleIdList[i]<<" ";
}
std::cout<<"---------------------------\n";

	//load data to triangleList
	triangleList = new Triangle[recordCount2];
	triangleListSize = recordCount2;
	for(int i=0; i<triangleListSize; i++){
		triangleList[i].coordinate[0] = coordinateList[triangleIdList[i*recordSize2]*recordSize1];
		triangleList[i].coordinate[1] = coordinateList[triangleIdList[i*recordSize2]*recordSize1+1];
		triangleList[i].coordinate[2] = coordinateList[triangleIdList[i*recordSize2+1]*recordSize1];
		triangleList[i].coordinate[3] = coordinateList[triangleIdList[i*recordSize2+1]*recordSize1+1];
		triangleList[i].coordinate[4] = coordinateList[triangleIdList[i*recordSize2+2]*recordSize1];
		triangleList[i].coordinate[5] = coordinateList[triangleIdList[i*recordSize2+2]*recordSize1+1];
		triangleList[i].coordinate[6] = triangleList[i].coordinate[0];
		triangleList[i].coordinate[7] = triangleList[i].coordinate[1];

		for(int j=0; j<8; j++)
			std::cout<<triangleList[i].coordinate[j]<<" ";

std::cout<<coordinateList[triangleIdList[i]*recordSize1]<<" ";
std::cout<<coordinateList[triangleIdList[i]*recordSize1+1]<<" ";
std::cout<<coordinateList[triangleIdList[i+1]*recordSize1]<<" ";
std::cout<<coordinateList[triangleIdList[i+1]*recordSize1+1]<<" ";
std::cout<<coordinateList[triangleIdList[i+2]*recordSize1]<<" ";
std::cout<<coordinateList[triangleIdList[i+2]*recordSize1+1]<<" ";

//		std::cout<<triangleIdList[i*recordSize2]<<" "<<triangleIdList[i*recordSize2+1]<<" "<<triangleIdList[i*recordSize2+2]<<"\n";
		std::cout<<"\n";
	}

	instreamTriangle.close();
	instreamVertex.close();

	delete [] coordinateList;
	delete [] triangleIdList;
}

void releaseDataSet(){
}

void drawGridLines(){
	int gBound[10] = {0,0, 1,0, 1,1, 0,1, 0,0};	
	//draw square & gridlines
	for(int i=0; i<10; i++){
		gBound[i]*scale;
		if(i%2==0)
			gBound[i] = gBound[i] * scale + originalX;
		else gBound[i] = gBound[i] * scale + originalY;
	}
	setcolor(RED);
	drawpoly(5, gBound);  // number of points, pointer to array 
	line(0 * scale + originalX,0.25 * scale + originalY,1 * scale + originalX,0.25 * scale + originalY);
	line(0 * scale + originalX,0.5 * scale + originalY,1 * scale + originalX,0.5 * scale + originalY);
	line(0 * scale + originalX,0.75 * scale + originalY,1 * scale + originalX,0.75 * scale + originalY);

	line(0.25 * scale + originalX,0 * scale + originalY,0.25 * scale + originalX,1 * scale + originalY);
	line(0.5 * scale + originalX,0 * scale + originalY,0.5 * scale + originalX,1 * scale + originalY);
	line(0.75 * scale + originalX,0 * scale + originalY,0.75 * scale + originalX,1 * scale + originalY);

}

void drawTriangles(){
std::cout<<"triangleListSize: "<<triangleListSize<<"\n";

//	int triangle[size][8];
	intTriangle *triangle = new intTriangle[triangleListSize];
	setcolor(CYAN);

	//Draw triangles
	for(int i=0; i<triangleListSize; i++){
		for(int j=0; j<8; j++){
			triangle[i].coordinate[j] = triangleList[i].coordinate[j]*scale;
			if(j%2==0) triangle[i].coordinate[j] = triangle[i].coordinate[j] + originalX;
			else triangle[i].coordinate[j] = triangle[i].coordinate[j] + originalY;
//			std::cout<<triangle[i].coordinate[j]<<" ";
//			std::cout<<triangleList[i].coordinate[j]*scale<<" ";
		}
//		std::cout<<"\n";
	}

	for(int i=0; i<triangleListSize; i++){
		for(int j=0; j<8; j++) 	std::cout<<triangle[i].coordinate[j]<<" ";
		std::cout<<"\n";
		drawpoly(4, triangle[i].coordinate);
	}
	delete [] triangleList;
	delete [] triangle;
}

int main(int argc, char **argv){
	int driver, mode;

	driver = DETECT; // autotect 
	mode = 0;
	initgraph(&driver, &mode, NULL);

	std::string path = argv[1];
	std::string fileStr1 = path + "mydata.smp";
	std::string fileStr2 = path + "mydata.ver";
	loadDataSet(fileStr1, fileStr2);
//	loadDataSet("./mydata.smp","./mydata.ver");

	drawGridLines();
	drawTriangles();

	getch(); // pause to admire 
	restorecrtmode();
}
