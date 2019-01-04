//g++ -std=gnu++11 -O3 common.cpp point.cpp edge.cpp boundingBox.cpp triangle.cpp linkList.cpp drawMesh.cpp draw.cpp -o draw -lgraph
#include <string>
#include "drawMesh.h"

//==============================================================
void draw(std::string pathFileName){
	std::string fileStr = pathFileName;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(6*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
std::cout<<"triangleNum: "<<triangleNum<<"\n";

	double *triangleCoorArr = new double[triangleNum*6];;

	fread(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);


	drawMesh *d = new drawMesh;
	int xPartNum = 4, yPartNum = 4;
	d->drawGridLines(xPartNum, yPartNum);
	if(triangleCoorArr!=NULL) d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 3);//CYAN
	delete d;
}
//==============================================================
int main(int argc, char **argv){
	std::string pathFileName = argv[1];
	draw(pathFileName);
	return 0;
}
