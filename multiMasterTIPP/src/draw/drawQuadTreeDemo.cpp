#include <iostream>
#include <time.h> 
#include <string>
#include <list>
#include <fstream>
#include <cmath>

#include "common.h"
#include "drawMesh.h"
#include "quadTree.h"
#include "io.h"


//g++ -std=gnu++11 -g point.cpp edge.cpp triangle.cpp boundingBox.cpp common.cpp io.cpp quadTree.cpp drawMesh.cpp drawQuadTreeDemo.cpp -o drawQuadTreeDemo -lgraph
//arguments: path=/home/kevin/, domainsize=1, number of points = 300, threshold = 50, groupId=0 (first group in 4 --> 0, 1, 2, 3)
//./drawQuadTreeDemo /home/kevin/ 1 300 50 0

using namespace std;


//==============================================================================
//read mapFile NorthCarolina_points.txt generated from python, all point coordinated in domain [0,0 - 1,1]
//input: NorthCarolina_points.txt
void readMapDataFromPython(double *&mapPointCoorArr, unsigned &mapPointNum, unsigned long long *&mapTriangleIdArr, unsigned &mapTriangleNum){
	mapPointCoorArr = NULL;
	mapTriangleIdArr = NULL;
	mapPointNum = mapTriangleNum = 0;
	std::string mapFileStr = "NorthCarolina_triangulation.txt";
	std::ifstream readMapFile(mapFileStr.c_str());
	std::string strItem;
	if(!readMapFile){
		std::cout<<"There is no filename "<<mapFileStr<<"\n";
		exit(1);
	}

	//read point data (for nc_inundation_v6c.ver)
	//first item is the the number of triangles
	readMapFile >> strItem;
	mapTriangleNum = atoll(strItem.c_str());
	if(mapTriangleNum==0) return;
	mapTriangleIdArr = new unsigned long long[mapTriangleNum*3];

	//second item is the the number of points
	readMapFile >> strItem;
	mapPointNum = atoll(strItem.c_str());
	if(mapPointNum==0) return;

	mapPointCoorArr = new double[mapPointNum*2];
	for(unsigned int i=0; i<mapPointNum; i++){
		readMapFile >> strItem;
		mapPointCoorArr[i*2] = atof(strItem.c_str());
		readMapFile >> strItem;
		mapPointCoorArr[i*2+1] = atof(strItem.c_str());
	}
	for(unsigned i=0; i<mapPointNum; i++) std::cout<<mapPointCoorArr[i*2]<<" "<<mapPointCoorArr[i*2+1]<<"\n";
//std::cout<<mapPointCoorArr[i*2]<<" "<<mapPointCoorArr[i*2+1]<<"\n";

	//read index data to mapTriangleIdArr (for nc_inundation_v6c.tri)
	for(unsigned i=0; i<mapTriangleNum; i++){
		//three column are three indices points
		readMapFile >> strItem;
		mapTriangleIdArr[3*i] = atoll(strItem.c_str());
		readMapFile >> strItem;
		mapTriangleIdArr[3*i+1] = atoll(strItem.c_str());
		readMapFile >> strItem;
		mapTriangleIdArr[3*i+2] = atoll(strItem.c_str());
	}
	//for(unsigned i=0; i<mapTriangleNum; i++) std::cout<<mapTriangleIdArr[i*3]<<" "<<mapTriangleIdArr[i*3+1]<<" "<<mapTriangleIdArr[i*3+2]<<"\n";
}


//==============================================================================
//read mapFile ADCIRC_MeshSmallData.txt to mapData and map to the domain [0,0 - 1,1]
//input: ADCIRC_MeshSmallData.txt
void readMapData(double *&mapPointCoorArr, unsigned &mapPointNum, unsigned long long *&mapTriangleIdArr, unsigned &mapTriangleNum){
	mapPointCoorArr = NULL;
	mapTriangleIdArr = NULL;
	mapPointNum = mapTriangleNum = 0;
	std::string mapFileStr = "ADCIRC_MeshSmallData.txt";
	std::ifstream readMapFile(mapFileStr.c_str());
	std::string strItem;
	if(!readMapFile){
		std::cout<<"There is no filename "<<mapFileStr<<"\n";
		exit(1);
	}

	//read point data (for nc_inundation_v6c.ver)
	//first item is the the number of triangles
	readMapFile >> strItem;
	mapTriangleNum = atoll(strItem.c_str());
	if(mapTriangleNum==0) return;
	mapTriangleIdArr = new unsigned long long[mapTriangleNum*3];

	//second item is the the number of points
	readMapFile >> strItem;
	mapPointNum = atoll(strItem.c_str());
	if(mapPointNum==0) return;

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


//=============================================================================
//dsitribute number of jobs/items (totalIitemNum) to processes (processNum)
//input: processNum, totalIitemNum
//output: itemNumArr, itemNumOffsetArr
//for example: compute number of triangles and offset should read for each process (triangleNumArr, offsetArr)
//=============================================================================
template <typename T>
void distribute(unsigned processNum, unsigned totalIitemNum, T *itemNumArr, T *itemNumOffsetArr){
	unsigned segmentSize;
	if(totalIitemNum < processNum){
		for(unsigned i=0; i<totalIitemNum; i++) itemNumArr[i] = 1;
		for(unsigned i=totalIitemNum; i<processNum; i++) itemNumArr[i] = 0;
		itemNumOffsetArr[0] = 0;
		for(unsigned i=1; i<=totalIitemNum; i++) itemNumOffsetArr[i] = itemNumOffsetArr[i-1] + 1;
		for(unsigned i=totalIitemNum+1; i<processNum; i++) itemNumOffsetArr[i] = itemNumOffsetArr[i-1];

	}else if((totalIitemNum % processNum) == 0){
		segmentSize = totalIitemNum/processNum;
		for(unsigned i=0; i<processNum; i++) itemNumArr[i] = segmentSize;
		itemNumOffsetArr[0] = 0;
		for(unsigned i=1; i<processNum; i++) itemNumOffsetArr[i] = itemNumOffsetArr[i-1] + segmentSize;
	}else{
		segmentSize = totalIitemNum/processNum;
		for(unsigned i=0; i<processNum; i++) itemNumArr[i] = segmentSize;

		unsigned leftOver = totalIitemNum - segmentSize*processNum;
		unsigned int index = 0;
		//add leftover to each element in array
		while(leftOver>0){
			itemNumArr[index]++;
			index++;
			leftOver--;
		}
		//itemNumOffsetArr
		itemNumOffsetArr[0] = 0;
		for(unsigned i=1; i<processNum; i++)
			itemNumOffsetArr[i] = itemNumOffsetArr[i-1] + itemNumArr[i-1];
	}
}

//==============================================================================
//generate points and triangles using qhul (rbox and )
//rbox 100 n D2 O0.5 > mydata.ver
//qdelaunay s Fv TO mydata.smp <mydata.ver
//now, mydata.ver (points) and mydata.smp (triangles) to draw.
//==============================================================================
void readTriangulation(std::string path, double *&pointCoorArr, unsigned &pointNum, unsigned long long *&triangleIdArr, unsigned &triangleNum){
	std::string pointFileStr = path + "mydata.ver";
	std::ifstream pointFile(pointFileStr.c_str());
	std::string strItem;
	if(!pointFile){
		std::cout<<"There is no filename "<<pointFileStr;
		exit(1);
	}
	//read point data (for mydata.ver)
	//the first item line is the point size
	pointFile >> strItem;
	unsigned pointRecordSize = atoi(strItem.c_str());
	//second item is the the number of points
	pointFile >> strItem;
	pointNum = atoll(strItem.c_str());
	cout<<"number of points: "<<pointNum<<"\n";
	pointCoorArr = new double[pointNum*pointRecordSize];

	for(unsigned i=0; i<pointNum; i++){
		pointFile >> strItem;
		pointCoorArr[i*pointRecordSize] = atof(strItem.c_str());
		pointFile >> strItem;
		pointCoorArr[i*pointRecordSize+1] = atof(strItem.c_str());
	}

	//read triangle data (mydata.smp)
	std::string triangleFileStr = path + "mydata.smp";
	std::ifstream triangleFile(triangleFileStr.c_str());
	if(!triangleFile){
		std::cout<<"There is no filename "<<triangleFileStr;
		exit(1);
	}

	//the first item line is the number of triangles
	triangleFile >> strItem;
	triangleNum = atoll(strItem.c_str());
	cout<<"number of triangles: "<<triangleNum<<"\n";
	triangleIdArr = new unsigned long long[triangleNum*3];
	for(unsigned i=0; i<triangleNum; i++){
		//skip the first column
		triangleFile >> strItem;
		triangleFile >> strItem;
		triangleIdArr[i*3] = atoi(strItem.c_str());
		triangleFile >> strItem;
		triangleIdArr[i*3+1] = atoi(strItem.c_str());
		triangleFile >> strItem;
		triangleIdArr[i*3+2] = atoi(strItem.c_str());
	}
	
	//test point data
	for(unsigned i=0; i<pointNum; i++){
		cout<<pointCoorArr[i*pointRecordSize]<<" "<<pointCoorArr[i*pointRecordSize+1]<<"\n";
	}
	//test triangle data
	for(unsigned i=0; i<triangleNum; i++){
		cout<<triangleIdArr[i*3]<<" "<<triangleIdArr[i*3+1]<<" "<<triangleIdArr[i*3+2]<<"\n";
	}
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
void drawTrianglesInQuadTree(quadTree qt, drawMesh *d){
	std::list<quadNode *> leafNodeList;
//	drawMesh *d = new drawMesh;
	qt.extractLeafNodes(leafNodeList);
	unsigned leafNodeNum = leafNodeList.size();

	//array of center points of all leafNode bounding boxes
	//will try to draw Z-order curve
	point *leafNodeBoxCenterPointArr= new point[leafNodeNum];

	unsigned index=0;
	unsigned pointNum=0;
	for(std::list<quadNode *>::iterator it=leafNodeList.begin(); it!=leafNodeList.end(); it++){
		quadNode *qNode = (*it);

		//draw triangles
		triangle *triangleArr;
		unsigned long triangleNum;
		converTriangleListToTriangleArr(qNode->triangleList, triangleArr, triangleNum);
		d->drawTriangleArr(triangleArr, triangleNum, 2);
		delete [] triangleArr;


		//draw box
		point lowPoint = qNode->box.getLowPoint();
		point highPoint = qNode->box.getHighPoint();
		lowPoint.setX(lowPoint.getX());
		lowPoint.setY(lowPoint.getY());
		highPoint.setX(highPoint.getX());
		highPoint.setY(highPoint.getY());
		d->drawBox(lowPoint, highPoint, 4);

		leafNodeBoxCenterPointArr[index].setX((lowPoint.getX() + highPoint.getX())/2);
		leafNodeBoxCenterPointArr[index].setY((lowPoint.getY() + highPoint.getY())/2);
		pointNum++;
		index++;
	}
	//d->drawLineArr(leafNodeBoxCenterPointArr, pointNum, 3);
	delete [] leafNodeBoxCenterPointArr;
}

//all triangles in domain are divided into 4 groups (or 4 segments)
//we will draw each group or each segment at a time
//================================================================
void drawAllQuadTreeTriangles(std::string path, float domainSize, unsigned threshold, unsigned groupId){
	unsigned pointNum, triangleNum;
	double *pointCoorArr=NULL;
	unsigned long long *triangleIdArr=NULL;
	//points and triangles generated from qhull
//	readTriangulation(path, pointCoorArr, pointNum, triangleIdArr, triangleNum);

	//points and triangles from mapFile nc_inundation_v6c.grd (ADCIRC)
	//readMapData(pointCoorArr, pointNum, triangleIdArr, triangleNum);

	//points and triangles from mapFile nc_inundation_v6c.grd (ADCIRC)
	readMapDataFromPython(pointCoorArr, pointNum, triangleIdArr, triangleNum);
	std::cout<<"number of points: "<<pointNum<<", number of triangles: "<<triangleNum<<"\n";


	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);
	for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

	//test data
//	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN

	unsigned processNum = 4;
	unsigned *triangleNumArr = new unsigned[processNum];
	unsigned *triangleNumOffsetArr = new unsigned[processNum];
	distribute(processNum, triangleNum, triangleNumArr, triangleNumOffsetArr);


/*for(unsigned i=0; i<processNum; i++)
	std::cout<<triangleNumArr[i]<<" "<<triangleNumOffsetArr[i]<<"\n";

for(unsigned i=0; i<triangleNumArr[0]; i++){
	std::cout<<triangleIdArr[i*3]<<" "<<triangleIdArr[i*3+1]<<" "<<triangleIdArr[i*3+2]<<"\n";
}

for(unsigned i=0; i<triangleNumArr[0]; i++){
	std::cout<<triangleCoorArr[i*6]<<" "<<triangleCoorArr[i*6+1]<<" "<<triangleCoorArr[i*6+2]<<" "<<triangleCoorArr[i*6+3]<<" "<<triangleCoorArr[i*6+4]<<" "<<triangleCoorArr[i*6+5]<<"\n";
}
*/
	drawMesh *d = new drawMesh;
	boundingBox domainBound(0, 0, 1.0, 1.0);
	quadTree qt(domainBound, threshold);
	qt.insertTriangles(&triangleIdArr[triangleNumOffsetArr[groupId]*3], &triangleCoorArr[triangleNumOffsetArr[groupId]*6], triangleNumArr[groupId]);
	qt.processQuadTree();
	drawTrianglesInQuadTree(qt, d);

	delete d;
	delete [] pointCoorArr;
	delete [] triangleIdArr;
	delete [] triangleCoorArr;
	delete [] triangleNumArr;
	delete [] triangleNumOffsetArr;
}


//================================================================
void drawWholeQuadTreeTriangles(std::string path, float domainSize, unsigned threshold){
	unsigned pointNum, triangleNum;
	double *pointCoorArr=NULL;
	unsigned long long *triangleIdArr=NULL;

	//points and triangles generated from qhull
//	readTriangulation(path, pointCoorArr, pointNum, triangleIdArr, triangleNum);

	//points and triangles from mapFile nc_inundation_v6c.grd (ADCIRC)
//	readMapData(pointCoorArr, pointNum, triangleIdArr, triangleNum);

	//points and triangles from mapFile nc_inundation_v6c.grd (ADCIRC)
	readMapDataFromPython(pointCoorArr, pointNum, triangleIdArr, triangleNum);
	std::cout<<"number of points: "<<pointNum<<", number of triangles: "<<triangleNum<<"\n";

	double *triangleCoorArr = new double[triangleNum*6];
	readCoorsFromTriangleIds(triangleIdArr, triangleNum, pointCoorArr, triangleCoorArr);
	for(unsigned i=0; i<triangleNum*6; i++) triangleCoorArr[i] /= domainSize;

	//test data
//	d->drawTriangleCoorArr(triangleCoorArr, triangleNum, 2);//GREEN

	drawMesh *d = new drawMesh;
	boundingBox domainBound(0, 0, 1.0, 1.0);
	quadTree qt(domainBound, threshold);
	qt.insertTriangles(triangleIdArr, triangleCoorArr, triangleNum);
	qt.processQuadTree();
	drawTrianglesInQuadTree(qt, d);

	delete d;
	delete [] pointCoorArr;
	delete [] triangleIdArr;
	delete [] triangleCoorArr;
}


//================================================================
int main(int argc, char **argv){
	if(argc<=2){// no arguments
		std::cout<<"You need to provide two arguments: path and domain size\n";
		exit(1);
	}
	else{
		std::string path = argv[1];
		float domainSize = atof(argv[2]);
		unsigned pointNum = atoi(argv[3]);
		unsigned threshold = atoi(argv[4]);
		unsigned groupId = atoi(argv[5]);
		cout<<"path: "<<path<<", and domain size: "<<domainSize<<", number of points: "<<pointNum<<", threshold: "<<threshold<<", groupId: "<<groupId<<"\n";
		//generate points and Delaunay triangulation
		//rbox 100 n D2 O0.5 > mydata.ver
		std::string cmdStr1 = "rbox " + std::to_string(pointNum) + " n D2 O0.5 > " + path + "mydata.ver";
		system(cmdStr1.c_str());
		//qdelaunay s Fv TO mydata.smp <mydata.ver
		std::string cmdStr2 = "qdelaunay s Fv TO " + path + "mydata.smp < " + path + "mydata.ver";
		system(cmdStr2.c_str());

		//draw group of quad trees from points and triangles that generated from qhull or North Carolina map (ADCIRC)
		drawAllQuadTreeTriangles(path, domainSize, threshold, groupId);
		//draw whole quad tree, from points and triangles that generated from qhull
//		drawWholeQuadTreeTriangles(path, domainSize, threshold);
	}

	return 0;
}

