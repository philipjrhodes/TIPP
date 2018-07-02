#include "delaunayLocal.h"

//============================================================================
delaunayLocal::delaunayLocal(std::string pathStr){

	vertexRecordSize = 2;
	partArr = NULL;
	tempCoorArr = NULL;
	tempPointIdArr = NULL;
	startIdPartArr = NULL;
	pointCoorArr = NULL;
	triangleSizeArr = NULL;
	triangleSizeOffsetArr = NULL;
	pointNumPartArr = NULL;
	pointNumPartOffsetArr = NULL;

	triangleList = NULL;
	temporaryTriangleList = NULL;
	storeTriangleList = NULL;

	path = pathStr;

	readTriangleData();
}

//============================================================================
//read all triangle data for active partitions, and other partition information
void delaunayLocal::readTriangleData(){

	//read meta data for temprary files
	std::string fileInfoStr = path + "tempTriangles.xfdl";
	std::ifstream triangleInfoFile(fileInfoStr.c_str());
	if(!triangleInfoFile){
		std::cout<<"non exist "<<fileInfoStr.c_str()<<std::endl;
		exit(1);
	}
	//read first line (1 number only) --> number of active partitions
	std::string strItem;
	triangleInfoFile >> strItem;
	partNum = atoi(strItem.c_str());

	//second line is active partition Ids
	partArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		partArr[i] = atoi(strItem.c_str());
	}

	//third line is the numbers of triangles which belong to active partitions
	triangleSizeArr = new int[partNum];//number of triangles belong to partitions
	totalTriangleSize = 0;
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeArr[i] = atoi(strItem.c_str());
		totalTriangleSize = totalTriangleSize + triangleSizeArr[i];
//std::cout<<triangleSizeArr[i]<<" ";
	}
//std::cout<<"\n";

	//fourth line is the offset array of previous line (triangle size for each partition)
	triangleSizeOffsetArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		triangleSizeOffsetArr[i] = atoi(strItem.c_str());
	}
	//fifth line is the numbers of new inserted points in each active partition
	pointNumPartArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartArr[i] = atoi(strItem.c_str());
	}
	//sixth line is the offset of previous line (point numbers of each active partition)
	pointNumPartOffsetArr = new int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		pointNumPartOffsetArr[i] = atoi(strItem.c_str());
	}
	//seventh line is xPartNum & yPartNum
	triangleInfoFile >> strItem;
	xPartNum = atoi(strItem.c_str());
	triangleInfoFile >> strItem;
	yPartNum = atoi(strItem.c_str());

	//eighth line is startIds for points in each active partition
	//skip initial points for each partition
	startIdPartArr = new unsigned long int[partNum];
	for(int i=0; i<partNum; i++){
		triangleInfoFile >> strItem;
		startIdPartArr[i] = atoi(strItem.c_str());
//std::cout<<startIdPartArr[i]<<" ";
	}

	triangleInfoFile.close();


	//read all coordinates of triangles
	std::string dataFileStr1 = path + "tempCoor.tri";
	FILE *f1 = fopen(dataFileStr1.c_str(), "rb");
	if(!f1){
		std::cout<<"not exist "<<dataFileStr1<<std::endl;
		return;
	}
	tempCoorArr = new double[totalTriangleSize*6];
	fread(tempCoorArr, sizeof(double), totalTriangleSize*6, f1);
	fclose(f1);

	//read all point ids of triangles
	std::string dataFileStr2 = path + "tempPointId.tri";
	FILE *f2 = fopen(dataFileStr2.c_str(), "rb");
	if(!f2){
		std::cout<<"not exist "<<dataFileStr2<<std::endl;
		return;
	}
	tempPointIdArr = new unsigned long int[totalTriangleSize*3];
	fread(tempPointIdArr, sizeof(double), totalTriangleSize*3, f2);
	fclose(f2);

/*
for(int i=0; i<totalTriangleSize; i++)
	std::cout<<tempPointIdArr[i*3]<<" "<<tempPointIdArr[i*3+1]<<" "<<tempPointIdArr[i*3+2]<<"========"<<tempCoorArr[i*6]<<" "<<tempCoorArr[i*6+1]<<" "<<tempCoorArr[i*6+2]<<" "<<tempCoorArr[i*6+3]<<" "<<tempCoorArr[i*6+4]<<" "<<tempCoorArr[i*6+5]<<"\n";
*/
}

//============================================================================
//read new inserted point coordinates from pointPart.ver 
//Depending on partId, read only coordinates belong to that partition
//based on pointNumPartOffsetArr, open file pointPart.ver (not including init points)
//index is order in partArr
void delaunayLocal::readPointCoor(unsigned int index){

	//pointPart.ver contains point coordinates of all partitions not including init points
	std::string fileStr = path + "pointPart.ver";
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	unsigned int partSize = pointNumPartArr[index];
	pointCoorArr = new double[partSize*vertexRecordSize];

	//Move to the right position of current partition on pointPart.ver
	unsigned int currentPos = pointNumPartOffsetArr[index];
	fseek(f, currentPos*sizeof(double)*vertexRecordSize, SEEK_CUR); // move to a currentPos

	//read all extension points (grid points on 4 edges of domain, incluing 3 corners)
	fread(pointCoorArr, sizeof(double), partSize*vertexRecordSize, f);
	fclose(f);
//for(int i=0; i<partSize; i++) std::cout<<pointCoorArr[2*i]<<" " <<pointCoorArr[2*i+1]<<"\n";
}

//============================================================================
//generate triangleList for partition order of index
void delaunayLocal::generateTriangles(unsigned int index){

	//triangleNum is number of triangles belongs to partId = partArr[index]
	int triangleNum = triangleSizeArr[index];

	int trianglePos = triangleSizeOffsetArr[index];

	for(int i=trianglePos; i<trianglePos+triangleNum; i++){
		point p1(tempCoorArr[i*6], tempCoorArr[i*6+1], tempPointIdArr[i*3]);
		point p2(tempCoorArr[i*6+2], tempCoorArr[i*6+3], tempPointIdArr[i*3+1]);
		point p3(tempCoorArr[i*6+4], tempCoorArr[i*6+5], tempPointIdArr[i*3+2]);
//std::cout<<pointIdArr[i*3]<<" "<<pointIdArr[i*3+1]<<" "<<pointIdArr[i*3+2]<<"\n";

		triangle *newTriangle = new triangle(p1, p2, p3);
		triangleNode *newTriangleNode = createNewNode(newTriangle);
		insertFront(triangleList, newTriangleNode);
	}
/*
if(my_rank==1){
triangleNode *head = triangleList;
while(head!=NULL){
	std::cout<<head->tri->p1.getId()<<" "<<head->tri->p2.getId()<<" "<<head->tri->p3.getId()<<"========"<<head->tri->p1.getX()<<" "<<head->tri->p1.getY()<<" "<<head->tri->p2.getX()<<" "<<head->tri->p2.getY()<<" "<<head->tri->p3.getX()<<" "<<head->tri->p3.getY()<<"\n";
	head = head->next;
}
}
*/
}

//==============================================================================
//determine boundingbox of current partition based on partId
boundingBox delaunayLocal::partBox(unsigned int partId){
	double xPartSize = 1.0/xPartNum;
	double yPartSize = 1.0/yPartNum;
	int gridPartX = partId % xPartNum;
	int gridPartY = partId / yPartNum;
	point lowPoint(gridPartX*xPartSize, gridPartY*yPartSize);
	point highPoint((gridPartX+1)*xPartSize, (gridPartY+1)*yPartSize);
//if(partId==13){
//std::cout<<xPartSize<<" "<<xPartSize<<"\n";
//std::cout<<"partID: "<<partId<<" "<<gridPartX*xPartSize<<" "<<gridPartY*yPartSize<<" "<<(gridPartX+1)*xPartSize<<" "<<(gridPartY+1)*yPartSize<<"\n";
//}
	return boundingBox(lowPoint, highPoint);
}

//===========================================================================
void delaunayLocal::printTriangleList(unsigned int partId){
	std::cout<<"triangleList size:"<<size(triangleList)<<"\n";
	std::cout<<"triangle Id List:"<<"\n";
	triangleNode *head = triangleList;
	while(head!=NULL){
		std::cout<<head->tri->p1.getX()<<" "<<head->tri->p1.getY()<<" " <<head->tri->p1.getId()<<"\n";
		std::cout<<head->tri->p2.getX()<<" "<<head->tri->p2.getY()<<" " <<head->tri->p2.getId()<<"\n";
		std::cout<<head->tri->p3.getX()<<" "<<head->tri->p3.getY()<<" " <<head->tri->p3.getId()<<"\n";
		head = head->next;
	}
}

//===========================================================================
//store finalized triangles (in storeTriangleList) to file returnStoreTriangleIds.tri.
//need store only point ids (of triangles) because we do have point coordinates in fullPointPart.ver
void delaunayLocal::processStoreTriangles(){
	unsigned int triangleNum = size(storeTriangleList);
	if(triangleNum==0) return;

	//transform storeTriangleList to array of ids
	unsigned long int *triangleIdArr = new unsigned long int[triangleNum*3];
	triangleNode *head = storeTriangleList;
	unsigned long int index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();
		index++;
		head=head->next;
	}
	removeLinkList(storeTriangleList);

	std::string fileStr = path + "returnStoreTriangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}	
	fwrite(triangleIdArr, sizeof(unsigned long int), triangleNum*3, f);
	fclose(f);
}

//===========================================================================
//process triangleList & temporaryTriangleList (tranform into array of triangles and send back to master node)
//These triangles (in temporaryTriangleList) have circumcircle intersected with border of current partition (i)
//therefore, those triangles will be belong to unfinished partitions, and keep for next triangulation of new partitions
void delaunayLocal::processTriangleList(){
	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);

	unsigned int triangleNum = size(triangleList);
	if(triangleNum==0) return;

	unsigned long int *triangleIdArr = new unsigned long int[triangleNum*3];
	//this is local pointCoorArr (different from the global one)
	double *pointCoorArr = new double[triangleNum*6];

	triangleNode *head = triangleList;
	unsigned long int index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		pointCoorArr[index*6] = head->tri->p1.getX();
		pointCoorArr[index*6+1] = head->tri->p1.getY();
		pointCoorArr[index*6+2] = head->tri->p2.getX();
		pointCoorArr[index*6+3] = head->tri->p2.getY();
		pointCoorArr[index*6+4] = head->tri->p3.getX();
		pointCoorArr[index*6+5] = head->tri->p3.getY();
		index++;
		head=head->next;
	}
	removeLinkList(triangleList);

	std::string fileStr = path + "returnTriangleIds.tri";
	FILE *f = fopen(fileStr.c_str(), "a");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}	
	fwrite(triangleIdArr, sizeof(unsigned long int), triangleNum*3, f);
	fclose(f);


	std::string fileStr1 = path + "returnTriangleCoors.tri";
	FILE *f1 = fopen(fileStr1.c_str(), "a");
	if(!f){
		std::cout<<"not exist "<<fileStr1<<std::endl;
		return;
	}	
	fwrite(pointCoorArr, sizeof(double), triangleNum*6, f1);
	fclose(f1);

}

//===========================================================================
//triangulate all active partitions sequentially
void delaunayLocal::partTriangulate(){
	for(int i=0; i<partNum; i++){
		//read new point coordinates for partition i
		readPointCoor(i);
		//generate triangles of current partition (partition i)
		generateTriangles(i);
		//use current triangles and new point coordinates to triangulate for partition i
		triangulate(i);

		//store finalized triangles (in storeTriangleList) to file. 
		//These triangles located inside current partition i, and never affected by any other triangulation
		processStoreTriangles();
		//These triangles (in temporaryTriangleList) have circumcircle intersected with border of current partition (i)
		//therefore, those triangles will be belong to unfinished partitions, and keep for next triangulation of new partitions
		processTriangleList();

		removeLinkList(triangleList);
		removeLinkList(temporaryTriangleList);
		removeLinkList(storeTriangleList);
	}
}

//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void delaunayLocal::triangulate(unsigned int index){//index is the order of partArr

	if(pointCoorArr==NULL) return;
	unsigned int partId = partArr[index];
std::cout<<"start triangulation from partId = "<<partId<<std::endl;

	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;
	double sweepLine = 0;


	//startId continues after ids of init triangulation
	unsigned long int startId = startIdPartArr[index];
	unsigned long int partPointId = startId;

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);
	int pointNumPartSize = pointNumPartArr[index];

	//sequentially insert point into delaunay
	for(unsigned int localPointIndex=0; localPointIndex<pointNumPartSize; localPointIndex++){
		p.setX(pointCoorArr[localPointIndex*2]);
		p.setY(pointCoorArr[localPointIndex*2+1]);
		p.setId(partPointId);
		partPointId++;
//if(index==1) std::cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
		sweepLine = p.getX();

		triangleNode *preNode = NULL;
		triangleNode *currNode = triangleList;
		bool goNext = true;
		while(currNode != NULL){
			//this triangle will never circumcircle the new point p
			if(currNode->tri->getFarestCoorX() < sweepLine){
				//if this triangle is within partition area, store it to storeTriangleList
				if(currNode->tri->inside(currPartBBox)){
 					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(storeTriangleList, exNode);
				}
				//store this triangle to temporaryTriangleList and use it for the next row
				else{
					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(temporaryTriangleList, exNode);
				}
				goNext = false;
			}
			else//the circumcircle of this triangle can contain the new point
			if(currNode->tri->circumCircleContains(p)){
				edge *edge1 = new edge(currNode->tri->getEdge1());
				edge *edge2 = new edge(currNode->tri->getEdge2());
				edge *edge3 = new edge(currNode->tri->getEdge3());

				edgeNode *eNode1 = createNewNode(edge1);
				edgeNode *eNode2 = createNewNode(edge2);
				edgeNode *eNode3 = createNewNode(edge3);

				insertFront(polygon, eNode1);
				insertFront(polygon, eNode2);
				insertFront(polygon, eNode3);

				removeNode(triangleList, preNode, currNode);
				goNext = false;
			}

			//go to the next element in the liskList
			if(goNext){
				preNode = currNode;
				currNode = currNode->next;
			}
			goNext = true;
		}

		//Find all bad edges from polygon, then remove bad edges later
		edgeNode *currEdgeNode1 = polygon;
		edgeNode *currEdgeNode2 = polygon;
		while(currEdgeNode1!=NULL){
			while(currEdgeNode2!=NULL){
				//two different edges in polygon but same geological edge
				if((currEdgeNode1!=currEdgeNode2)&&(*(currEdgeNode1->ed) == *(currEdgeNode2->ed))){
					edge *newEdge = new edge(*currEdgeNode1->ed);
					edgeNode *newEdgeNode = createNewNode(newEdge);
					insertFront(badEdges, newEdgeNode);
				}
				currEdgeNode2 = currEdgeNode2->next;
			}
			currEdgeNode1 = currEdgeNode1->next;
			currEdgeNode2 = polygon;
		}

		//remove all bad edges from polygon
		edgeNode *badEdgeNode = badEdges;
		edgeNode *preEdgeNode = NULL;
		edgeNode *currEdgeNode = polygon;
		while(badEdgeNode!=NULL){
			while(currEdgeNode!=NULL){
				if(*(badEdgeNode->ed) == *(currEdgeNode->ed)){
					removeNode(polygon, preEdgeNode, currEdgeNode);
					break;
				}
				else{
					preEdgeNode = currEdgeNode;
					currEdgeNode = currEdgeNode->next;
				}
			}
			badEdgeNode = badEdgeNode->next;
			
			preEdgeNode = NULL;
			currEdgeNode = polygon;
		}

		//remove all bad edges
		removeLinkList(badEdges);


		//polygon now is a hole. With insert point p, form new triangles and insert into triangleList
		edgeNode *scanEdgeNode = polygon;
		while(scanEdgeNode!=NULL){
			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			newTriangle->computeCenterRadius();
			if(newTriangle->colinear()||newTriangle->isBad())
				delete newTriangle;
			else{
				triangleNode *newTriangleNode = createNewNode(newTriangle);
				insertFront(triangleList, newTriangleNode);
			}
		scanEdgeNode = scanEdgeNode->next;
		}

		//delete polygon, badTriangle
		removeLinkList(polygon);
	}
	delete [] pointCoorArr;
	pointCoorArr = NULL;
	std::cout<<"done triangulation from partId = "<<partId<<std::endl;
}


//===========================================================================
delaunayLocal::~delaunayLocal(){
	delete [] partArr;
	delete [] startIdPartArr;
	delete [] pointNumPartArr;
	delete [] pointNumPartOffsetArr;
}
