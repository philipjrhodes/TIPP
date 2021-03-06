#include <iostream>
#include <string>
#include <list>
#include "common.h"
#include "drawMesh.h"
#include "io.h"
#include <fstream>
#include <cmath>

//read files
#include <sys/types.h>
#include <dirent.h>


//g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp common.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph
//edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp

//./drawDomain ../dataSources/mapData/ ../dataSources/mapData/ 8

using namespace std;


//==============================================================================
//read mapFile nc_inundation_v6c.grd to mapData and map to the domain [0,0 - 1,1]
//input: mapFile nc_inundation_v6c.grd
//output: triangleIdsArr --> nc_inundation_v6c.tri
//output: pointCoorArr --> nc_inundation_v6c.ver
void readMapData(double *&mapPointCoorArr, unsigned &mapPointNum, unsigned long long *&mapTriangleIdArr, unsigned &mapTriangleNum){
	std::string mapFileStr = "nc_inundation_v6c.grd";
	std::ifstream readMapFile(mapFileStr.c_str());
	std::string strItem;
	if(!readMapFile){
		std::cout<<"There is no filename " << mapFileStr;
		exit(1);
	}

	//read point data (for nc_inundation_v6c.ver)
	//skip the first item line because it is the file name
	readMapFile >> strItem;
	//second item is the the number of triangles
	readMapFile >> strItem;
	mapTriangleNum = atoll(strItem.c_str());
	mapTriangleIdArr = new unsigned long long[mapTriangleNum*3];

    //third item of input contains number of points
	readMapFile >> strItem;
	mapPointNum = atoll(strItem.c_str());

	double maxX, minX, maxY, minY, x, y;
	readMapFile >> strItem;
	readMapFile >> strItem;
	x = atof(strItem.c_str());
	readMapFile >> strItem;
	y = atof(strItem.c_str());
	readMapFile >> strItem;
	maxX = x; maxY = y;
	minX = x; minY = y;

	mapPointCoorArr = new double[mapPointNum*2];
	mapPointCoorArr[0] = x;
	mapPointCoorArr[1] = y;

	for(unsigned int i=1; i<mapPointNum; i++){
		readMapFile >> strItem;
		readMapFile >> strItem;
		x = atof(strItem.c_str());
		mapPointCoorArr[i*2] = x;
		if(maxX < x) maxX = x;
		if(minX > x) minX = x;

		readMapFile >> strItem;
		y = atof(strItem.c_str());
		mapPointCoorArr[i*2+1] = y;
		if(maxY < y) maxY = y;
		if(minY > y) minY = y;
		readMapFile >> strItem;
	}
	for(unsigned i=mapPointNum-10; i<mapPointNum; i++) std::cout<<mapPointCoorArr[i*2]<<" "<<mapPointCoorArr[i*2+1]<<"\n";

	double deltaX = maxX-minX;
	double deltaY = maxY-minY;
	for(unsigned int i=0; i<mapPointNum; i++){
		mapPointCoorArr[i*2] = (mapPointCoorArr[i*2]-minX)/deltaX;
		mapPointCoorArr[i*2+1] = (mapPointCoorArr[i*2+1]-minY)/deltaY;

//std::cout<<mapPointCoorArr[i*2]<<" "<<mapPointCoorArr[i*2+1]<<"\n";
	}

	//read index data to mapTriangleIdArr (for nc_inundation_v6c.tri)
	for(unsigned i=0; i<mapTriangleNum; i++){
		//skip first two columns because they are order and 3 points
		readMapFile >> strItem;
		readMapFile >> strItem;
		//the last three column are three indices points
		readMapFile >> strItem;
		mapTriangleIdArr[3*i] = atoll(strItem.c_str())-1;
		readMapFile >> strItem;
		mapTriangleIdArr[3*i+1] = atoll(strItem.c_str())-1;
		readMapFile >> strItem;
		mapTriangleIdArr[3*i+2] = atoll(strItem.c_str())-1;
		//-1 means the array start from 0 but the dataset starts from 1
	}
	for(unsigned i=mapTriangleNum-10; i<mapTriangleNum; i++) std::cout<<mapTriangleIdArr[i*3]<<" "<<mapTriangleIdArr[i*3+1]<<" "<<mapTriangleIdArr[i*3+2]<<"\n";
}

//==================================================================================
double triangleArea(double x1, double y1, double x2, double y2, double x3, double y3){
	return fabs((x1-x3)*(y2-y1) - (x1-x2)*(y3-y1))/2;
}

//==============================================================================
bool insideBoundingBox(point p, boundingBox b){
	double xVal = p.getX();
	double yVal = p.getY();
	point lowPoint = b.getLowPoint();
	point highPoint = b.getHighPoint();
	double xLow = lowPoint.getX();
	double yLow = lowPoint.getY();
	double xHigh = highPoint.getX();
	double yHigh = highPoint.getY();

	if((xVal>xLow)&&(xVal<xHigh)&&(yVal>yLow)&&(yVal<yHigh)) return true;
	else return false;
}

//================================================================
void readTriangles(triangle *&triangleArr, unsigned int &triangleNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(sizeof(triangle)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	triangleArr = new triangle[triangleNum];

	fread(triangleArr, triangleNum, sizeof(triangle), f);
	fclose(f);
}

/*
//================================================================
void readTriangleCoors(double *&triangleCoorArr, unsigned int &triangleNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(6*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	triangleCoorArr = new double[triangleNum*6];

	fread(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);
}
*/

//================================================================
void readAllCoors(double *&pointCoorArr, unsigned int &pointNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	pointNum = ftell(f)/(2*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	pointCoorArr = new double[pointNum*2];

	fread(pointCoorArr, pointNum*2, sizeof(double), f);
	fclose(f);
}

/*
//================================================================
void readTriangleIds(unsigned long long *&triangleIdArr, unsigned int &triangleNum, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	triangleNum = ftell(f)/(3*sizeof(unsigned long long)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	triangleIdArr = new unsigned long long[triangleNum*3];

	fread(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
}
*/

//================================================================
void readTriangleIdsFromBigFile(unsigned long long *&triangleIdArr, unsigned triangleNum, unsigned long long offset, std::string fullPath){
	std::string fileStr = fullPath;
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}
	fseek(f, offset*3*sizeof(unsigned long long), SEEK_SET); // seek back to beginning of file
	triangleIdArr = new unsigned long long[triangleNum*3];

	fread(triangleIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);
}


//================================================================
void readCoorsFromTriangleIds(unsigned long long *triangleIdArr, unsigned int &triangleNum, double *pointCoorArr, double *triangleCoorArr){
	//fill triangleCoorArr from triangleIdArr and pointCoorArr
	for(int triangleId=0; triangleId<triangleNum; triangleId++){
		triangleCoorArr[triangleId*6] = pointCoorArr[triangleIdArr[triangleId*3]*2];
		triangleCoorArr[triangleId*6+1] = pointCoorArr[triangleIdArr[triangleId*3]*2+1];
		triangleCoorArr[triangleId*6+2] = pointCoorArr[triangleIdArr[triangleId*3+1]*2];
		triangleCoorArr[triangleId*6+3] = pointCoorArr[triangleIdArr[triangleId*3+1]*2+1];
		triangleCoorArr[triangleId*6+4] = pointCoorArr[triangleIdArr[triangleId*3+2]*2];
		triangleCoorArr[triangleId*6+5] = pointCoorArr[triangleIdArr[triangleId*3+2]*2+1];
	}
}

//================================================================
void readCoorsFromTriangleArr(triangle *triangleArr, unsigned int &triangleNum, double *triangleCoorArr){
	//fill triangleCoorArr from triangleIdArr and pointCoorArr
	for(unsigned int triangleId=0; triangleId<triangleNum; triangleId++){
		triangleCoorArr[triangleId*6] = triangleArr[triangleId].p1.getX();
		triangleCoorArr[triangleId*6+1] = triangleArr[triangleId].p1.getY();
		triangleCoorArr[triangleId*6+2] = triangleArr[triangleId].p2.getX();
		triangleCoorArr[triangleId*6+3] = triangleArr[triangleId].p2.getY();
		triangleCoorArr[triangleId*6+4] = triangleArr[triangleId].p3.getX();
		triangleCoorArr[triangleId*6+5] = triangleArr[triangleId].p3.getY();
	}
}


//============================================================================
void writeTriangleIds_ADCIRC(unsigned long long *triangleIdArr, unsigned long long triangleIdArrSize, std::string outputPath){
	if((triangleIdArrSize==0)||(triangleIdArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(triangleIdArr, triangleIdArrSize*3, sizeof(unsigned long long), f);
	fclose(f);
	std::cout<<"write to file "<<outputPath<<", with "<<triangleIdArrSize<<" triangles\n";
}

//============================================================================
void writePointCoorArr_ADCIRC(double *pointCoorArr, unsigned pointNum, std::string outputPath){
	if((pointNum==0)||(pointCoorArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(pointCoorArr, pointNum*2, sizeof(double), f);
	fclose(f);
	std::cout<<"write to file "<<outputPath<<", with "<<pointNum<<" points\n";
}

//============================================================================
void writeTriangleCoorArr_ADCIRC(double *triangleCoorArr, unsigned triangleNum, std::string outputPath){
	if((triangleNum==0)||(triangleCoorArr==NULL)) return;
	FILE *f = fopen(outputPath.c_str(), "wb");
	if(!f){
		std::cout<<"not success to open "<<outputPath<<std::endl;
		exit(1);
	}
	//std::cout<<"append boundary data to "<<outputPath<<"\n";
	fwrite(triangleCoorArr, triangleNum*6, sizeof(double), f);
	fclose(f);
	std::cout<<"write to file "<<outputPath<<", with "<<triangleNum<<" points\n";
}


//==================================================================================
void drawMap_ADCIRC(std::string dstPath){
	double *pointCoorArr;
	unsigned pointNum;
	unsigned long long *triangleIdArr;
	unsigned triangleNum;
	readMapData(pointCoorArr, pointNum, triangleIdArr, triangleNum);
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);

	writePointCoorArr_ADCIRC(pointCoorArr, pointNum, dstPath + "pointCoor_ADCIRC.ver");
	delete [] pointCoorArr;
	writeTriangleIds_ADCIRC(triangleIdArr, triangleNum, dstPath + "triangleIds_ADCIRC.tri");
	delete [] triangleIdArr;

	drawMesh *d = new drawMesh;
	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
	delete d;

	writeTriangleCoorArr_ADCIRC(triangleCoorArr, triangleNum, dstPath + "triangleCoor_ADCIRC.tri");
	delete [] triangleCoorArr;
}

//from filename, return boundingBox
//===================================================================
boundingBox fileNameToBox(std::string fileName, unsigned domainSize){
	double xLow = 0;
	double yLow = 0;
	double xHigh = domainSize;
	double yHigh = domainSize;

	for(unsigned i=1; i<fileName.length(); i++){
		switch (fileName[i]){
			case '0':	yLow = (yHigh+yLow)/2; 
						xHigh = (xHigh+xLow)/2;break;
			case '1':	xLow = (xHigh+xLow)/2;
						yLow = (yHigh+yLow)/2;break;
			case '2':	xHigh = (xHigh+xLow)/2;
						yHigh = (yHigh+yLow)/2;break;
			case '3':	xLow = (xHigh+xLow)/2;
						yHigh = (yHigh+yLow)/2;
		}
	}
	return boundingBox(xLow/domainSize, yLow/domainSize, xHigh/domainSize, yHigh/domainSize);
}

/*//===================================================================
//std::list<std::string> list_dir(std::string path){
//input: a directory
//output: a list og strings (filenames in the directory)
int getdir (std::string dir, std::list<std::string> &files){
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << errno << ") opening " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}
*/
//===================================================================
void drawAQuadNodeInBigFile(std::string srcPath, std::string dstPath, unsigned int domainSize, unsigned leafNodeId){
	//read all quad node info
	std::string fileInfoStr = dstPath + "triangleIds_ZOrder.xfdl";
	std::ifstream quadTreeInfoFile(fileInfoStr.c_str());
	if(!quadTreeInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first line --> read number of quad nodes
	quadTreeInfoFile >> strItem;
	unsigned quadNodeNum = atoi(strItem.c_str());
	//second line --> read all number of triangles in leaf nodes
	unsigned *triangleNumArr = new unsigned[quadNodeNum];
	for(unsigned i=0; i<quadNodeNum; i++){
		quadTreeInfoFile >> strItem;
		triangleNumArr[i] = atoi(strItem.c_str());
		//std::cout<<triangleNumArr[i]<<" ";
	}
	//std::cout<<"\n";
	//third line --> read all ofsset triangleNum
	unsigned long long *triangleNumOffsetArr = new unsigned long long[quadNodeNum];
	for(unsigned i=0; i<quadNodeNum; i++){
		quadTreeInfoFile >> strItem;
		triangleNumOffsetArr[i] = atoi(strItem.c_str());
		//std::cout<<triangleNumOffsetArr[i]<<" ";
	}
	//std::cout<<"\n";

	//next quadNodeNum lines are the boundingbox of all leaf nodes
	double lowX, lowY, highX, highY;
	point lowPoint, highPoint;
	boundingBox *boxLeafNodeArr = new boundingBox[quadNodeNum];
	for(unsigned i=0; i<quadNodeNum; i++){
		//read coordinates of current box
		quadTreeInfoFile >> strItem;
		lowX = stod(strItem.c_str());

		quadTreeInfoFile >> strItem;
		lowY = stod(strItem.c_str());

		quadTreeInfoFile >> strItem;
		highX = stod(strItem.c_str());

		quadTreeInfoFile >> strItem;
		highY = stod(strItem.c_str());
		
		lowPoint.set(point(lowX, lowY));
		highPoint.set(point(highX, highY));

		boxLeafNodeArr[i].setLowPoint(lowPoint);
		boxLeafNodeArr[i].setHighPoint(highPoint);
	}
	std::cout<<boxLeafNodeArr[leafNodeId].getLowPoint();
	std::cout<<boxLeafNodeArr[leafNodeId].getHighPoint();

	//draw a leaf Node in quad tree
	drawMesh *d = new drawMesh;
	if((leafNodeId < 0) || (leafNodeId > quadNodeNum)){
		std::cout<<"number of leaf nodes is ="<<quadNodeNum<<"\n";
		std::cout<<leafNodeId<<" is out of range\n";
	}else{
		double *allPointCoorArr;
		unsigned int pointNum;
		readAllCoors(allPointCoorArr, pointNum, srcPath + "fullPointPart.ver");

		unsigned long long *triangleIdArr;
		unsigned triangleNum = triangleNumArr[leafNodeId];
		readTriangleIdsFromBigFile(triangleIdArr, triangleNum, triangleNumOffsetArr[leafNodeId], dstPath + "triangleIds_ZOrder.tri");

		double *triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);
		for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

		d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
		point lowPoint = boxLeafNodeArr[leafNodeId].getLowPoint();
		point highPoint = boxLeafNodeArr[leafNodeId].getHighPoint();
		std::cout<<lowPoint;
		std::cout<<highPoint;
		lowPoint.setX(lowPoint.getX()/domainSize);
		lowPoint.setY(lowPoint.getY()/domainSize);
		highPoint.setX(highPoint.getX()/domainSize);
		highPoint.setY(highPoint.getY()/domainSize);
		d->drawBox(lowPoint, highPoint, 4);

		delete [] allPointCoorArr;
		delete [] triangleIdArr;
	}

	delete [] triangleNumArr;
	delete [] triangleNumOffsetArr;
	delete [] boxLeafNodeArr;
	delete d;
}


//===================================================================
void readQuadTreeInfo(unsigned *&triangleNumArr, unsigned long long *&triangleNumOffsetArr, boundingBox *&boxLeafNodeArr, unsigned &leafNodeNum, std::string dstPath){
	//read all quad node info
	std::string fileInfoStr = dstPath + "triangleIds_ZOrder.xfdl";
	std::ifstream quadTreeInfoFile(fileInfoStr.c_str());
	if(!quadTreeInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;
	//first line --> read number of quad nodes
	quadTreeInfoFile >> strItem;
	leafNodeNum = atoi(strItem.c_str());
	//second line --> read all number of triangles in leaf nodes
	triangleNumArr = new unsigned[leafNodeNum];
	for(unsigned i=0; i<leafNodeNum; i++){
		quadTreeInfoFile >> strItem;
		triangleNumArr[i] = atoi(strItem.c_str());
	}

	//third line --> read all ofsset triangleNum
	triangleNumOffsetArr = new unsigned long long[leafNodeNum];
	for(unsigned i=0; i<leafNodeNum; i++){
		quadTreeInfoFile >> strItem;
		triangleNumOffsetArr[i] = atoi(strItem.c_str());
	}

	//next quadNodeNum lines are the boundingbox of all leaf nodes
	double lowX, lowY, highX, highY;
	point lowPoint, highPoint;
	boxLeafNodeArr = new boundingBox[leafNodeNum];
	for(unsigned i=0; i<leafNodeNum; i++){
		//read coordinates of current box
		quadTreeInfoFile >> strItem;
		lowX = std::stod(strItem.c_str());

		quadTreeInfoFile >> strItem;
		lowY = std::stod(strItem.c_str());

		quadTreeInfoFile >> strItem;
		highX = std::stod(strItem.c_str());

		quadTreeInfoFile >> strItem;
		highY = std::stod(strItem.c_str());
		
		lowPoint.set(point(lowX, lowY));
		highPoint.set(point(highX, highY));

		boxLeafNodeArr[i].setLowPoint(lowPoint);
		boxLeafNodeArr[i].setHighPoint(highPoint);
	}
}

//===================================================================
//load files triangleIds_ZOrder.xfdl and triangleIds_ZOrder.tri
void drawAllQuadNodesInBigFile(std::string srcPath, std::string dstPath, std::string triangleInfoFile, std::string pointCoorFile, std::string triangleIdFile, unsigned int domainSize){
	drawMesh *d = new drawMesh;

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + pointCoorFile);

	unsigned long long *allTriangleIdArr;
	unsigned allTriangleNum;
	readTriangleIds(allTriangleIdArr, allTriangleNum, dstPath + triangleIdFile);

	double *allTriangleCoorArr = new double[allTriangleNum*6];
	readCoorsFromTriangleIds(allTriangleIdArr, allTriangleNum, allPointCoorArr, allTriangleCoorArr);
	for(unsigned i=0; i<allTriangleNum*6; i++) allTriangleCoorArr[i] /= domainSize;

	unsigned *triangleNumArr;
	unsigned long long *triangleNumOffsetArr;
	boundingBox *boxLeafNodeArr;
	unsigned leafNodeNum;
	readQuadTreeInfo(triangleNumArr, triangleNumOffsetArr, boxLeafNodeArr, leafNodeNum, dstPath);

	for(unsigned quadNodeId=0; quadNodeId<leafNodeNum; quadNodeId++){
		d->drawTriangleCoorArr(&allTriangleCoorArr[triangleNumOffsetArr[quadNodeId]*6], triangleNumArr[quadNodeId], 2);//GREEN

		boundingBox box = boxLeafNodeArr[quadNodeId];
		point lowPoint = box.getLowPoint();
		lowPoint.setX(lowPoint.getX()/domainSize);
		lowPoint.setY(lowPoint.getY()/domainSize);
		point highPoint = box.getHighPoint();
		highPoint.setX(highPoint.getX()/domainSize);
		highPoint.setY(highPoint.getY()/domainSize);
		d->drawBox(lowPoint, highPoint, 4);
	}

	delete d;
	delete [] allPointCoorArr;
	delete [] allTriangleCoorArr;
	delete [] allTriangleIdArr;
	delete [] triangleNumArr;
	delete [] triangleNumOffsetArr;
	delete [] boxLeafNodeArr;
}



//===================================================================
void drawAllQuadNodes(std::string srcPath, std::string dstPath, std::string pointCoorFile, unsigned int domainSize){
	drawMesh *d = new drawMesh;

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + pointCoorFile);

	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	double *triangleCoorArr;

	std::list<std::string> files;
	std::list<std::string> processFiles;
	getdir(dstPath, files);
	std::string s;
	unsigned count = 0;
	//count number of 0*.tri files
	for(std::list<std::string>::iterator it=files.begin(); it!=files.end(); it++){
		s = (*it);
		if(s[0]=='0'){
			count++;
			processFiles.push_back(s);
		}
	}
	std::cout<<"Number of files 0*:"<<count<<"\n";


//	unsigned N=80;
//	std::string fileNameArr[80] = {"000", "0000", "001", "013", "0020", "0021", "022", "0022", "0030", "0033", "0100", "0110", "0112","0113", "0121", "0122", "0123", "0201", "0210", "00230", "0231", "00231", "0232", "00232", "00233", "0301", "0310", "00310", "0311", "00311", "00312", "00313", "0320", "0321", "0330", "01010", "01011", "01012", "01013", "01020", "01021", "01022", "01023", "01030", "01031", "01032", "01033", "01110", "01111", "01112", "01113", "01200", "01201", "01202", "01203", "02000", "02001", "02002", "02003", "02020", "02021", "02022", "02023", "02030", "02031", "02032", "02033", "02110", "02111", "02112", "02113", "02120", "02121", "02122", "02123", "02130", "02131", "02132", "02133", "02300"};

	for(std::list<std::string>::iterator it=processFiles.begin(); it!=processFiles.end(); it++){
		//std::string currPath = dstPath + fileNameArr[i] + ".tri";
		std::string fileName = (*it);
		std::string currPath = dstPath + fileName;
		readTriangleIds(triangleIdArr, triangleNum, currPath);
		triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);
		//scale down triangleCoorArr (in doamin 1x1)
		for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

		d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN

		boundingBox box = fileNameToBox(fileName, domainSize);
		d->drawBox(box.getLowPoint(), box.getHighPoint(), 4);
		std::cout<<box.getLowPoint().getX()<<" "<<box.getLowPoint().getY()<<" "<<box.getHighPoint().getX()<<" "<<box.getHighPoint().getY()<<"\n";

		delete [] triangleIdArr;
		delete [] triangleCoorArr;
	}

	delete d;
	delete [] allPointCoorArr;
	processFiles.clear();
	files.clear();
}



//===================================================================
void drawOnePartition(std::string srcPath, std::string dstPath, std::string fileName, unsigned partId, unsigned int domainSize, unsigned partNumBothSize){
	drawMesh *d = new drawMesh;
	d->	drawGridLines(point(0,0), point(1, 1), partNumBothSize, partNumBothSize, 4);

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + "fullPointPart.ver");

	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	double *triangleCoorArr;

	std::string currPath = generateFileName(partId, dstPath + fileName, partNumBothSize*partNumBothSize, ".tri");
	readTriangleIds(triangleIdArr, triangleNum, currPath);
	triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);

	//scale down triangleCoorArr (in doamin 1x1)
	for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
	delete [] triangleIdArr;
	delete [] triangleCoorArr;

	delete d;
	delete [] allPointCoorArr;

}


//===================================================================
void drawAllPartition(std::string srcPath, std::string dstPath, unsigned int domainSize, unsigned partNumBothSize){
	drawMesh *d = new drawMesh;
	d->	drawGridLines(point(0,0), point(1, 1), partNumBothSize, partNumBothSize, 4);

	double *allPointCoorArr;
	unsigned int pointNum;
	readAllCoors(allPointCoorArr, pointNum, srcPath + "fullPointPart.ver");

	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	double *triangleCoorArr;
//	for(unsigned partId=32; partId<=40; partId++){
	for(unsigned partId=0; partId<partNumBothSize*partNumBothSize; partId++){
		std::string currPath = generateFileName(partId, dstPath + "/triangleIds", partNumBothSize*partNumBothSize, ".tri");
		readTriangleIds(triangleIdArr, triangleNum, currPath);
		triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleIds(triangleIdArr, triangleNum, allPointCoorArr, triangleCoorArr);

		//scale down triangleCoorArr (in doamin 1x1)
		for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

		std::cout<<"draw partition "<<currPath<<"\n";
		d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleIdArr;
		delete [] triangleCoorArr;
	}

	delete d;
	delete [] allPointCoorArr;
}

//===================================================================
//Based on all triangles ids in triangleIds.tri, draw all triangles (include triangles with grid points)
void drawTriangleArr(std::string srcPath, std::string dstPath, unsigned int domainSize){
	//read fullPointPart.ver to pointCoorArr
	std::string fileStr = srcPath + "/fullPointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int pointNum = ftell(f)/(2*sizeof(double)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file
	double *pointCoorArr = new double[pointNum*2];

	fread(pointCoorArr, pointNum*2, sizeof(double), f);
	fclose(f);

	//read triangle Ids (edges)
	std::string fileStr1 = dstPath + "/triangleIds.tri";
//	std::string fileStr1 = dstPath + "/triangleIds_ZOrder.tri";
//	std::string fileStr1 = path + "/domainTriangleIds.tri";
	//std::string fileStr1 = path + "/returnAllStoreTriangleIds10.tri";
	f = fopen(fileStr1.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}
	fseek(f, 0, SEEK_END); // seek to end of file
	unsigned int triangleNum = ftell(f)/(3*sizeof(unsigned long long)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	unsigned long long *pointIdArr = new unsigned long long[triangleNum*3];
	fread(pointIdArr, triangleNum*3, sizeof(unsigned long long), f);
	fclose(f);


//for(int i=0; i<triangleNum; i++)
//	std::cout<<"["<<pointIdArr[i*3]<<","<<pointIdArr[i*3+1]<<","<<pointIdArr[i*3+2]<<"] ";

	double triangleAreas = 0;

	//build storeTriangleArr
	triangle *triangleArr = new triangle[triangleNum];
std::cout<<"pointNum: "<<pointNum<<", and number of triangles: "<<triangleNum<<"\n";
	for(int i=0; i<triangleNum; i++){
		unsigned long long pointId1 = pointIdArr[i*3];
		unsigned long long pointId2 = pointIdArr[i*3+1];
		unsigned long long pointId3 = pointIdArr[i*3+2];

		triangleArr[i].p1.setId(pointId1);
		triangleArr[i].p2.setId(pointId2);
		triangleArr[i].p3.setId(pointId3);

		triangleArr[i].p1.setX(pointCoorArr[pointId1*2]/domainSize);
		triangleArr[i].p1.setY(pointCoorArr[pointId1*2+1]/domainSize);

		triangleArr[i].p2.setX(pointCoorArr[pointId2*2]/domainSize);
		triangleArr[i].p2.setY(pointCoorArr[pointId2*2+1]/domainSize);

		triangleArr[i].p3.setX(pointCoorArr[pointId3*2]/domainSize);
		triangleArr[i].p3.setY(pointCoorArr[pointId3*2+1]/domainSize);
//std::cout<<triangleArr[i].p1.getId()<<" "<<triangleArr[i].p1.getX()<<" "<<triangleArr[i].p1.getY()<<"\n";
//std::cout<<triangleArr[i].p2.getId()<<" "<<triangleArr[i].p2.getX()<<" "<<triangleArr[i].p2.getY()<<"\n";
//std::cout<<triangleArr[i].p3.getId()<<" "<<triangleArr[i].p3.getX()<<" "<<triangleArr[i].p3.getY()<<"\n";

		triangleAreas += triangleArea(triangleArr[i].p1.getX(), triangleArr[i].p1.getY(), triangleArr[i].p2.getX(), triangleArr[i].p2.getY(), triangleArr[i].p3.getX(), triangleArr[i].p3.getY());
	}
	std::cout<<"triangleAreas: "<<triangleAreas<<std::endl;

	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(domainSize, domainSize);
	d->drawTriangleArr(triangleArr, triangleNum, 2);//GREEN
	delete d;

	delete [] pointCoorArr;
	delete [] pointIdArr;
	delete [] triangleArr;
}

//================================================================
void drawDomainTriangles(std::string path, unsigned int domainSize){
	std::string fileStr = path + "/domainTriangles.tri";

	unsigned int triangleNum;
	double *triangleCoorArr;
	readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

	for(unsigned int i=0; i<triangleNum*6; i++){
		triangleCoorArr[i] = triangleCoorArr[i]/domainSize;
	}

	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(domainSize, domainSize);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	delete d;

	delete [] triangleCoorArr;
}

//================================================================
void drawCoarseBoundaryTriangles(unsigned int domainSize, std::string path){
	std::string fileStr = path + "/boundaryCoors.tri";
	double *triangleCoorArr;
	unsigned int triangleNum;
	readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

	for(unsigned int i=0; i<triangleNum; i++){
		triangleCoorArr[i*6] /= domainSize;
		triangleCoorArr[i*6+1] /= domainSize;
		triangleCoorArr[i*6+2] /= domainSize;
		triangleCoorArr[i*6+3] /= domainSize;
		triangleCoorArr[i*6+4] /= domainSize;
		triangleCoorArr[i*6+5] /= domainSize;
//std::cout<<triangleCoorArr[i*6]<<" "<<triangleCoorArr[i*6+1]<<" "<<triangleCoorArr[i*6+2]<<" "<<triangleCoorArr[i*6+3]<<" "<<triangleCoorArr[i*6+4]<<" "<<triangleCoorArr[i*6+5]<<"\n";
	}

	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(domainSize, domainSize);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	delete d;

	delete [] triangleCoorArr;

}

//================================================================
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawDomainActivePartitions(std::string path){

	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());

	unsigned int *activeCoarsePartIdArr = new unsigned int[activeCoarsePartNum];
	//second line: read active partition ids (coarsePartition Ids)
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}

	unsigned int *activeCoarsePartSizeArr = new unsigned int[activeCoarsePartNum];
	//third line stores number of init triangles belong to active partitions (coarsePartitions)
	//take number of init triangles of current coarse partition in the array of active coarse partitions
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartSizeArr[i] = atoi(strItem.c_str());
	}
	initTriangleInfoFile.close();

	FILE *f;
	unsigned int coarsePartId;
	unsigned int triangleNum;
	std::string fileStr;
	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);
	//read coordinates and point Ids
	for(int i=0; i<activeCoarsePartNum; i++){
		coarsePartId = activeCoarsePartIdArr[i];
		triangleNum = activeCoarsePartSizeArr[i];


		fileStr = generateFileName(i, path + "/tempCoorCoarseParts", activeCoarsePartNum, ".tri");
		FILE *f = fopen(fileStr.c_str(), "rb");
		if(!f){
			std::cout<<"not exist "<<fileStr<<std::endl;
			return;
		}
		double *triangleCoorArr = new double[triangleNum*6];
		fread(triangleCoorArr, sizeof(double), triangleNum*6, f);
		fclose(f);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

		delete [] triangleCoorArr;
	}

	delete d;
	delete [] activeCoarsePartIdArr;
	delete [] activeCoarsePartSizeArr;
}
//================================================================
//Draw init triangles of all active coarse partitions
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
void drawInitTrianglesOneCoarsePart(std::string path){
	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile.close();

	drawMesh *d = new drawMesh;
//	d->drawGridLines(point(0, 0), point(1.0, 1.0), 4, 4, 4);
	d->drawGridLines(point(0.5, 0.5), point(0.75, 0.75), 4, 4, 4);

	std::string fileStr;
	for(int i=3; i<activeCoarsePartNum; i++){
		fileStr = generateFileName(i, path + "/initTrianglesCoors", activeCoarsePartNum, ".tri");

		unsigned int triangleNum;
		double *triangleCoorArr;
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);
		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

		delete [] triangleCoorArr;
	}
	delete d;
}

//================================================================
//Draw init triangles of all active coarse partitions
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawInitTrianglesAllCoarseParts(std::string path){
	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile.close();

	drawMesh *d = new drawMesh;
	d->drawGridLines(point(0, 0), point(1.0, 1.0), 4, 4, 4);
	d->drawGridLines(point(0.5, 0.5), point(0.75, 0.75), 4, 4, 4);

	std::string fileStr;
	for(int i=0; i<activeCoarsePartNum; i++){
		fileStr = generateFileName(i, path + "/initTrianglesCoors", activeCoarsePartNum, ".tri");

		unsigned int triangleNum;
		double *triangleCoorArr;
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleCoorArr;
	}
	delete d;
}

//================================================================
//Draw active fine partitions
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawActiveFineParts(std::string path){
	drawMesh *d = new drawMesh;
	d->	oldDrawGridLines(4, 4);

	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());

	unsigned int *activeCoarsePartIdArr = new unsigned int[activeCoarsePartNum];
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}
	//skip 4 numbers
	for(int i=0; i<activeCoarsePartNum; i++) initTriangleInfoFile >> strItem;

	//read two last number --> xCoarsePartNum, yCoarsePartNum

	initTriangleInfoFile >> strItem;
	int xCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile >> strItem;
	int yCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile.close();

	//draw grid lines for all active partitions
	for(unsigned i=0; i<activeCoarsePartNum; i++){
		boundingBox bb = findPart(activeCoarsePartIdArr[i], point(0.0, 0.0), point(1.0, 1.0), 4, 4);
		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), 4, 4, 4);
	}

	for(int i=0; i<activeCoarsePartNum; i++){
		std::string fileStr = generateFileName(activeCoarsePartIdArr[i], path + "/tempCoorFineParts", xCoarsePartNum*yCoarsePartNum, ".tri");
		unsigned int triangleNum;
		double *triangleCoorArr;
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleCoorArr;	
	}

	delete d;
	delete [] activeCoarsePartIdArr;
}

//================================================================
//Draw one active fine partitions
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
void drawOneActiveFineParts(std::string path){
	drawMesh *d = new drawMesh;

	//Read information from tempTriangles.xfdl
	std::string fileInfoStr = path + "/tempTrianglesCoarseParts.xfdl";
	std::ifstream initTriangleInfoFile(fileInfoStr.c_str());
	if(!initTriangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	std::string strItem;

	//first line read number of active coarse partitions
	initTriangleInfoFile >> strItem;
	int activeCoarsePartNum = atoi(strItem.c_str());

	unsigned int *activeCoarsePartIdArr = new unsigned int[activeCoarsePartNum];
	for(int i=0; i<activeCoarsePartNum; i++){
		initTriangleInfoFile >> strItem;
		activeCoarsePartIdArr[i] = atoi(strItem.c_str());
	}
	//skip 4 numbers
	for(int i=0; i<activeCoarsePartNum; i++) initTriangleInfoFile >> strItem;

	//read two last number --> xCoarsePartNum, yCoarsePartNum

	initTriangleInfoFile >> strItem;
	int xCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile >> strItem;
	int yCoarsePartNum = atoi(strItem.c_str());
	initTriangleInfoFile.close();

	//draw grid lines for all active partitions
	for(unsigned i=3; i<activeCoarsePartNum; i++){
		boundingBox bb = findPart(activeCoarsePartIdArr[i], point(0.0, 0.0), point(1.0, 1.0), 4, 4);
		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), 4, 4, 4);
	}

	for(int i=3; i<activeCoarsePartNum; i++){
		std::string fileStr = generateFileName(activeCoarsePartIdArr[i], path + "/tempCoorFineParts", xCoarsePartNum*yCoarsePartNum, ".tri");
		unsigned int triangleNum;
		double *triangleCoorArr;
		readTriangleCoors(triangleCoorArr, triangleNum, fileStr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
		delete [] triangleCoorArr;	
	}

	delete d;
	delete [] activeCoarsePartIdArr;
}

//================================================================
//Draw returnStoredTriangles and returnTriangles
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
void drawReturnTriangles(std::string path){
	drawMesh *d = new drawMesh;
//	d->	oldDrawGridLines(4, 4);
	d->drawGridLines(point(0.5, 0.5), point(0.75, 0.75), 4, 4, 4);

	//draw returnTriangles (boundary triangles)
	unsigned int triangleNum;
	double *triangleCoorArr;
	readTriangleCoors(triangleCoorArr, triangleNum, path + "/returnAllTriangleCoors10.tri");
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	delete [] triangleCoorArr;



	//draw interior triangles from returnStoreTriangles
	unsigned long long *triangleIdArr;
	readTriangleIds(triangleIdArr, triangleNum, path + "/returnAllStoreTriangleIds10.tri");

	//read fullPointPart.ver to pointCoorArr
	double *pointCoorArr;
	unsigned int pointNum;
	readAllCoors(pointCoorArr, pointNum, path + "/fullPointPart.ver");
	triangleCoorArr = new double[triangleNum*6];

	boundingBox bb = findPart(10, point(0.0, 0.0), point(1.0, 1.0), 4, 4);
	std::cout<<bb.getLowPoint();
	std::cout<<bb.getHighPoint();

	//fill triangleCoorArr from triangleIdArr and pointCoorArr
	for(int triangleId=0; triangleId<triangleNum; triangleId++){
//std::cout<<triangleIdArr[triangleId*3]<<" "<<triangleIdArr[triangleId*3+1]<<" "<<triangleIdArr[triangleId*3+2]<<"\n";
		triangleCoorArr[triangleId*6] = pointCoorArr[triangleIdArr[triangleId*3]*2];
		triangleCoorArr[triangleId*6+1] = pointCoorArr[triangleIdArr[triangleId*3]*2+1];

//		if(!insideBoundingBox(point(triangleCoorArr[triangleId*6], triangleCoorArr[triangleId*6+1]), bb)){
//			std::cout<<"point "<<triangleCoorArr[triangleId*6]<<" "<<triangleCoorArr[triangleId*6]<<" is outside the partition "<<"\n";
//		}
		triangleCoorArr[triangleId*6+2] = pointCoorArr[triangleIdArr[triangleId*3+1]*2];
		triangleCoorArr[triangleId*6+3] = pointCoorArr[triangleIdArr[triangleId*3+1]*2+1];

		triangleCoorArr[triangleId*6+4] = pointCoorArr[triangleIdArr[triangleId*3+2]*2];
		triangleCoorArr[triangleId*6+5] = pointCoorArr[triangleIdArr[triangleId*3+2]*2+1];
	}
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

	delete [] triangleCoorArr;
	delete [] triangleIdArr;
	delete [] pointCoorArr;

	delete d;
}

//================================================================
//Draw interior triangles (returnAllStoreTriangleIdsXX.tri) and boundary triangles (boundaryTrianglesXX.tri) after the first stage of domain triangulation
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
//if you want to scale up, change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp (for coarsePartId = 10 only)
void drawOneInteriorBoundary(unsigned int coarsePartId, std::string path){
	drawMesh *d = new drawMesh;

	//draw interior triangles of a coarse partition
	unsigned int triangleNum;
	unsigned long long *triangleIdArr;
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;

	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);
//	d->drawGridLines(point(0.5, 0.5), point(0.75, 0.75), 4, 4, 4);


	std::string fileStr = generateFileName(coarsePartId, path + "/returnAllStoreTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	readTriangleIds(triangleIdArr, triangleNum, fileStr);

	//read fullPointPart.ver to pointCoorArr
	double *pointCoorArr;
	unsigned int pointNum;
	readAllCoors(pointCoorArr, pointNum, path + "/fullPointPart.ver");
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);

	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

	delete [] triangleCoorArr;
	delete [] triangleIdArr;
	delete [] pointCoorArr;

	//draw boundary triangles of a coarse partition
	fileStr = generateFileName(coarsePartId, path + "/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
	triangle *triangleArr;
	readTriangles(triangleArr, triangleNum, fileStr);
	triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

	delete [] triangleCoorArr;
	delete [] triangleArr;

	delete d;
}

//================================================================
//Draw all interior and boundary triangles of all active coarse partitions
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawAllInteriorBoundary(std::string path){
	unsigned int activePartArr[] = {0, 2, 8, 10};
	drawMesh *d = new drawMesh;

	//draw interior triangles of a coarse partition
	unsigned int triangleNum;
	unsigned long long *triangleIdArr;
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	std::string fileStr;

	d->drawGridLines(point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum, 4);

	for(int i=0; i<4; i++){
		unsigned int coarsePartId = activePartArr[i];
		boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);

/*		fileStr = generateFileName(coarsePartId, path + "/returnAllStoreTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
		readTriangleIds(triangleIdArr, triangleNum, fileStr);

		//read fullPointPart.ver to pointCoorArr
		double *pointCoorArr;
		unsigned int pointNum;
		readAllCoors(pointCoorArr, pointNum, path + "/fullPointPart.ver");
		double *triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

		delete [] triangleCoorArr;
		delete [] triangleIdArr;
		delete [] pointCoorArr;
*/
		//draw boundary triangles of a coarse partition
		fileStr = generateFileName(coarsePartId, path + "/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
		triangle *triangleArr;
		readTriangles(triangleArr, triangleNum, fileStr);
		double *triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN

		delete [] triangleCoorArr;
		delete [] triangleArr;
	}

	delete d;
}

//================================================================
//Draw inactive and the returned triangles for the next stage of domain
//The default scale is (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
void drawNewDomainTrianglesInNextStage(std::string path){
	triangle *triangleArr;
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	unsigned int triangleNum;

	//draw inactive triangles
	std::string fileStr = path + "/inActiveTrangle.tri";
	readTriangles(triangleArr, triangleNum, fileStr);
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

	drawMesh *d = new drawMesh;
	d->drawGridLines(point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum, 4);
	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 2);//GREEN
	delete [] triangleCoorArr;
	delete [] triangleArr;


	//draw returned triangles (bounadry triangles)
	unsigned int activePartArr[] = {0, 2, 8, 10};
	for(int i=0; i<4; i++){
		unsigned int coarsePartId = activePartArr[i];
		boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
		d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);

		//draw boundary triangles of a coarse partition
		fileStr = generateFileName(coarsePartId, path + "/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
		triangle *triangleArr;
		readTriangles(triangleArr, triangleNum, fileStr);
		double *triangleCoorArr = new double[triangleNum*6];
		readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

		d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 1);//BLUE

		delete [] triangleCoorArr;
		delete [] triangleArr;
	}

	delete d;
}

//================================================================
void drawBoundaryTriangles(unsigned int coarsePartId, std::string path){
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	drawMesh *d = new drawMesh;

	//draw returned triangles (bounadry triangles)
	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);

	//draw boundary triangles of a coarse partition
	std::string fileStr = generateFileName(coarsePartId, path + "/boundaryTriangles", xCoarsePartNum*yCoarsePartNum, ".tri");
	triangle *triangleArr;
	unsigned int triangleNum;
	readTriangles(triangleArr, triangleNum, fileStr);
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleArr(triangleArr, triangleNum, triangleCoorArr);

	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 1);//BLUE

	delete [] triangleCoorArr;
	delete [] triangleArr;
	delete d;
}

//================================================================
void drawReturnAllStoreTriangles(unsigned int coarsePartId, std::string path){
	unsigned int xCoarsePartNum = 4;
	unsigned int yCoarsePartNum = 4;
	drawMesh *d = new drawMesh;
	d->drawGridLines(point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum, 4);

	boundingBox bb = findPart(coarsePartId, point(0.0, 0.0), point(1.0, 1.0), xCoarsePartNum, yCoarsePartNum);
	d->drawGridLines(bb.getLowPoint(), bb.getHighPoint(), xCoarsePartNum, yCoarsePartNum, 4);

	//draw returnAllStoreTriangles
	unsigned long long *triangleIdArr;
	unsigned int triangleNum;
	std::string fileStr = generateFileName(coarsePartId, path + "/returnAllStoreTriangleIds", xCoarsePartNum*yCoarsePartNum, ".tri");
	readTriangleIds(triangleIdArr, triangleNum, fileStr);

	//read fullPointPart.ver to pointCoorArr
	double *pointCoorArr;
	unsigned int pointNum;
	readAllCoors(pointCoorArr, pointNum, path + "/fullPointPart.ver");
	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);

	d->drawCoordinateTriangles(triangleCoorArr, triangleNum, 3);//CYAN

	delete [] triangleCoorArr;
	delete [] triangleIdArr;
	delete [] pointCoorArr;
	delete d;
}


//================================================================
int main(int argc, char **argv){

	if(argc<=3){// no arguments
		std::cout<<"You need to provide two arguments: path\n";
		exit(1);
	}
	else{
		std::string srcPath = argv[1];
		std::string dstPath = argv[2];
		unsigned int domainSize = atoi(argv[3]);

//		drawAllQuadNodesInBigFile(srcPath, dstPath, "triangleIds_ZOrder.xfdl", "fullPointPart.ver", "triangleIds_ZOrder.tri", domainSize);

//		drawMap_ADCIRC(dstPath);
//		drawAQuadNodeInBigFile(srcPath, dstPath, domainSize, 33);
//		drawAllQuadNodes(srcPath, dstPath, "fullPointPart.ver", domainSize);
//		drawAllQuadNodes(srcPath, dstPath, "pointCoor_ADCIRC.ver", domainSize);

		//change (scale = 450; originalX = 20; originalY = 20;) in drawMesh.cpp
		drawTriangleArr(srcPath, dstPath, domainSize);
//		drawCoarseBoundaryTriangles(domainSize, path);
//		drawDomainTriangles(path, domainSize);
//		drawDomainActivePartitions(path);
//		drawInitTrianglesAllCoarseParts(path);
//		drawActiveFineParts(path);

		//change (scale = 1300; originalX = -580; originalY = -580;) in drawMesh.cpp
//		drawInitTrianglesOneCoarsePart(path);
//		drawOneActiveFineParts(path);
//		drawReturnTriangles(path);
//		drawOneInteriorBoundary(10, path);
//		drawAllInteriorBoundary(path);
//		drawNewDomainTrianglesInNextStage(path);
//		drawBoundaryTriangles(7, path);
//		drawReturnAllStoreTriangles(7, path);
//		drawBoundaryTriangles(7, path);
	}

	return 0;
}
