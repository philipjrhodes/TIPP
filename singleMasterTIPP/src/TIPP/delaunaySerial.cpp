#include "delaunaySerial.h"

//============================================================================
delaunaySerial::delaunaySerial(unsigned xPartNumInput, unsigned yPartNumInput, unsigned domainSizeInput, std::string srcPath){
	xPartNum = xPartNumInput;
	yPartNum = yPartNumInput;
	domainSize = domainSizeInput;
	path = srcPath;

	pointCoorArr=NULL;
	pointNum = 0;

	initTriangleList = NULL;
	interiorTriangleList = NULL;
	boundaryTriangleList = NULL;
}

//============================================================================
void delaunaySerial::processSerial(unsigned partId, triangleNode *&initTriangleListInput, triangleNode *&returnInteriorTriangleList, triangleNode *&returnBoundaryTriangleList){
	returnInteriorTriangleList = NULL;
	returnBoundaryTriangleList = NULL;

	initTriangleList = initTriangleListInput;
	readPointCoor(partId);
	triangulate(partId);
	delete [] pointCoorArr;

	returnInteriorTriangleList = interiorTriangleList;
	returnBoundaryTriangleList = boundaryTriangleList;
	initTriangleListInput = NULL;

	initTriangleList = NULL;
	interiorTriangleList = NULL;
	boundaryTriangleList = NULL;
}

//==============================================================================
int int_compar(const void *p1, const void *p2){
  double x1 = ((point*)p1)->getX();
  double x2 = ((point*)p2)->getX();
  return x1 > x2;
}

//============================================================================
//read point coordinates from pointPart.ver 
//Depending on partId, read only coordinates belong tos that partition
void delaunaySerial::readPointCoor(unsigned partId){
	std::string fileStr = generateFileName(partId, path + "/pointPart", xPartNum*yPartNum, ".ver");
	FILE *f = fopen(fileStr.c_str(), "rb");
	if(!f){
		std::cout<<"not exist "<<fileStr<<std::endl;
		return;
	}

	fseek(f, 0, SEEK_END); // seek to end of file
	pointNum = ftell(f)/(sizeof(point)); // get current file pointer
	fseek(f, 0, SEEK_SET); // seek back to beginning of file

	//pointCoorArr is an array of points
	if(pointNum!=0){
		pointCoorArr = new point[pointNum];
		fread(pointCoorArr, pointNum, sizeof(point), f);
		fclose(f);
	}
//	qsort(pointCoorArr, pointNum, sizeof(point), int_compar);
}

//==============================================================================
//determine boundingbox of current partition based on partId
boundingBox delaunaySerial::partBox(unsigned int partId){
	double xPartSize = domainSize/xPartNum;
	double yPartSize = domainSize/yPartNum;
	unsigned int gridPartX = partId % xPartNum;
	unsigned int gridPartY = partId / yPartNum;
	point lowPoint(gridPartX*xPartSize, gridPartY*yPartSize);
	point highPoint((gridPartX+1)*xPartSize, (gridPartY+1)*yPartSize);
	return boundingBox(lowPoint, highPoint);
}


//==============================================================================
//Delaunay triangulation
//input: an array of point (pointArr), domain/partition (squareBBox), initialTriangleList
//output: a list of triangles which are triangulated: boundaryTriangleList, interiorTriangleList
//initialTriangleList is all triangles in the partition as initialization
//interiorTriangleList contains triangles stay wholly inside partition and are finalized
//==============================================================================
void delaunaySerial::triangulate(unsigned int partId){

	if((pointCoorArr==NULL) || (pointNum==0)) return;
	unsigned int count=0;

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);
	triangulatePartition(currPartBBox, pointCoorArr, pointNum, initTriangleList, interiorTriangleList, boundaryTriangleList);

	releaseMemory(pointCoorArr);

	std::cout<<"done triangulation with partId = "<<partId<<", interiorTriangleSize: "<<size(interiorTriangleList)<<", boundaryTriangleSize: "<<size(boundaryTriangleList)<<", bad triangles:"<<count<<std::endl;}


/*
//==============================================================================
//Delaunay triangulation
//input: an array of point (coorPointArr)
//output: a list of triangles which are triangulated
//==============================================================================
void delaunaySerial::triangulate(unsigned partId){

	if((pointCoorArr==NULL) || (pointNum==0)) return;
	unsigned int count=0;

	triangleNode *temporaryTriangleList=NULL;
	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;
	double sweepLine = 0;

	//determine boundingbox of current partition based on partId
	boundingBox currPartBBox = partBox(partId);

	//sequentially insert point into delaunay
	for(unsigned long long localPointIndex=0; localPointIndex<pointNum; localPointIndex++){
		p = pointCoorArr[localPointIndex];

		sweepLine = p.getX();

		triangleNode *preNode = NULL;
		triangleNode *currNode = initTriangleList;
		bool goNext = true;
		while(currNode != NULL){
			//this triangle will never circumcircle the new point p
			if(currNode->tri->getFarestCoorX() < sweepLine){
				//if this triangle is within partition area, store it to storeTriangleList
				if(currNode->tri->inside(currPartBBox)){
 					triangleNode *exNode = extractNode(initTriangleList, preNode, currNode);
					insertFront(interiorTriangleList, exNode);
				}
				//store this triangle to temporaryTriangleList and use it for the next row
				else{
					triangleNode *exNode = extractNode(initTriangleList, preNode, currNode);
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

				removeNode(initTriangleList, preNode, currNode);
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
				insertFront(initTriangleList, newTriangleNode);
			}
		scanEdgeNode = scanEdgeNode->next;
		}

		//delete polygon, badTriangle
		removeLinkList(polygon);
	}

	//extract all interior triangles out of initialTriangleList to interiorTriangleList
	triangleNode *currNode = initTriangleList;
	triangleNode *preNode = NULL;
	while(currNode != NULL){
		if(currNode->tri->inside(currPartBBox)){
			triangleNode *exNode = extractNode(initTriangleList, preNode, currNode);
			insertFront(interiorTriangleList, exNode);
		}else{
			preNode = currNode;
			currNode = currNode->next;
		}
	}

	//join temporaryTriangleList and initialTriangleList to initialTriangleList
	if(temporaryTriangleList!=NULL)	addLinkList(temporaryTriangleList, initTriangleList);
	//join initialTriangleList to boundaryTriangleList

	if(initTriangleList!=NULL) addLinkList(initTriangleList, boundaryTriangleList);

	std::cout<<"done triangulation with partId = "<<partId<<", interiorTriangleSize: "<<size(interiorTriangleList)<<", boundaryTriangleSize: "<<size(boundaryTriangleList)<<", bad triangles:"<<count<<std::endl;
}
*/
