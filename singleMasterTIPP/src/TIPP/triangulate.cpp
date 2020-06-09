#include <iostream>
#include "triangulate.h"

//==============================================================================
//Delaunay triangulation for a partition
//input: an array of point (pointArr), domain/partition (partBBox), initialTriangleList
//output: a list of triangles which are triangulated: boundaryTriangleList, interiorTriangleList
//initialTriangleList is all triangles in the partition as initialization
//interiorTriangleList contains triangles stay wholly inside partition and are finalized
//==============================================================================
void triangulatePartition(boundingBox partBBox, point *pointArr, unsigned pointNum, triangleNode *&initialTriangleList, triangleNode *&interiorTriangleList, triangleNode *&boundaryTriangleList){

	//list of triangles that are not currently processed delaunay, will be processed by other partitions
	triangleNode *temporaryTriangleList=NULL;

	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;
	point p;
	double sweepLine = 0;

	//sequentially insert point into delaunay
	for(unsigned long long localPointIndex=0; localPointIndex<pointNum; localPointIndex++){
		p = pointArr[localPointIndex];

		sweepLine = p.getX();

		triangleNode *preNode = NULL;
		triangleNode *currNode = initialTriangleList;
		bool goNext = true;
		while(currNode != NULL){
			//this triangle will never circumcircle the new point p
			if(currNode->tri->getFarestCoorX() < sweepLine){
				//if this triangle is within partition area, store it to interiorTriangleList
				if(currNode->tri->inside(partBBox)){
 					triangleNode *exNode = extractNode(initialTriangleList, preNode, currNode);
					insertFront(interiorTriangleList, exNode);
				}
				//store this triangle to temporaryTriangleList and use it for the next row
				else{
					triangleNode *exNode = extractNode(initialTriangleList, preNode, currNode);
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

				removeNode(initialTriangleList, preNode, currNode);
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

		//polygon now is a hole. With insert point p, form new triangles and insert into initialTriangleList
		edgeNode *scanEdgeNode = polygon;
		while(scanEdgeNode!=NULL){
			triangle *newTriangle = new triangle(scanEdgeNode->ed->p1, scanEdgeNode->ed->p2, p);
			newTriangle->computeCenterRadius();
			if(newTriangle->collinear()||newTriangle->isBad())
				delete newTriangle;
			else{
				triangleNode *newTriangleNode = createNewNode(newTriangle);
				insertFront(initialTriangleList, newTriangleNode);
			}
		scanEdgeNode = scanEdgeNode->next;
		}

		//delete polygon, badTriangle
		removeLinkList(polygon);
	}


	//extract all interior triangles out of initialTriangleList to interiorTriangleList
	triangleNode *currNode = initialTriangleList;
	triangleNode *preNode = NULL;
	while(currNode != NULL){
		if(currNode->tri->inside(partBBox)){
			triangleNode *exNode = extractNode(initialTriangleList, preNode, currNode);
			insertFront(interiorTriangleList, exNode);
		}else{
			preNode = currNode;
			currNode = currNode->next;
		}
	}

	//join temporaryTriangleList and initialTriangleList to initialTriangleList
	if(temporaryTriangleList!=NULL)	addLinkList(temporaryTriangleList, initialTriangleList);
	//join initialTriangleList to boundaryTriangleList
	if(initialTriangleList!=NULL) addLinkList(initialTriangleList, boundaryTriangleList);
}


//==============================================================================
//Delaunay triangulation for domain
//input: an array of point (pointArr), triangleList
//output: a list of triangles which are triangulated: triangleList
//triangleList is all triangles in the partition as initialization
//==============================================================================
void triangulateDomain(point *pointArr, unsigned pointNum, triangleNode *&triangleList){

	//list of triangles that are not currently processed delaunay, will be processed by other partitions
	triangleNode *temporaryTriangleList=NULL;
	point p;
	double sweepLine = 0;
	edgeNode *polygon = NULL;
	edgeNode *badEdges = NULL;

	//sequentially insert point into delaunay
	for(unsigned long long localPointIndex=0; localPointIndex<pointNum; localPointIndex++){
		p = pointArr[localPointIndex];
		sweepLine = p.getX();

		triangleNode *preNode = NULL;
		triangleNode *currNode = triangleList;
		bool goNext = true;

		while(currNode != NULL){
			//this triangle will never circumcircle the new point p
			if(currNode->tri->getFarestCoorX() < sweepLine){
				//store this triangle to temporaryTriangleList and use it for the next row
					triangleNode *exNode = extractNode(triangleList, preNode, currNode);
					insertFront(temporaryTriangleList, exNode);
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
				if(*(badEdgeNode->ed) == *(currEdgeNode->ed))
					removeNode(polygon, preEdgeNode, currEdgeNode);
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
				if(newTriangle->collinear()||newTriangle->isBad())
					delete newTriangle;
				else{
					triangleNode *newTriangleNode = createNewNode(newTriangle);
					insertFront(triangleList, newTriangleNode);
				}
			scanEdgeNode = scanEdgeNode->next;
		}

	removeLinkList(polygon);
	}

	//join temporaryTriangleList to triangleList
	if(temporaryTriangleList!=NULL)	 addLinkList(temporaryTriangleList, triangleList);
}


//==============================================================================
//for the partitions that have no inserted points (partArr[partId].finish = true; partArr[partId].active = false;)
//The interior triangles would be no change after triangulation whole domain, therefore, we should extract these triangles out of initial mesh
void extractInteriorTriangles(boundingBox partBBox, triangleNode *&triangleList, triangleNode *&interiorTriangleList){

	triangleNode *currNode = triangleList;
	triangleNode *preNode = NULL;

	while(currNode != NULL)
		if(currNode->tri->inside(partBBox)){
			triangleNode *exNode = extractNode(triangleList, preNode, currNode);
			insertFront(interiorTriangleList, exNode);
		}else{
			preNode = currNode;
			currNode = currNode->next;
		}
}
