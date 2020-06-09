#include <iostream>
#include "linkList.h"

//g++ common.cpp point.cpp edge.cpp triangle.cpp linkList.cpp -o ll

//functions for edgeNode
edgeNode *createNewNode(edge *e){
	edgeNode *eNode = new edgeNode;
	eNode->ed = e;
	eNode->next = NULL;
	return eNode;
}

void insertFront(edgeNode *&head, edgeNode *newNode){
	if(head == NULL){
		head = newNode;
		head->next = NULL;
	}
	else{
		newNode->next = head;
		head = newNode;
	}
}

void removeNode(edgeNode *&head, edgeNode *&preP, edgeNode *&currP){
	if(currP==head){//delete first node
		currP = currP->next;
		edgeNode *temp = head;
		head = currP;
		delete temp;
	}
	else{ //delete middle or last node
		edgeNode *temp = currP;
		preP->next = currP->next;
		currP = preP->next;
		delete temp;
	}
}

edgeNode *extractNode(edgeNode *&head, edgeNode *preP, edgeNode *&currP){
	edgeNode *temp = NULL;
	if(currP==head){//delete first node
		currP = currP->next;
		temp = head;
		head = currP;
	}
	else{ //delete middle or last node
		temp = currP;
		preP->next = currP->next;
		currP = preP->next;
	}
	return temp;
}

void removeLinkList(edgeNode *&head){

	edgeNode *scanNode = head;
	while(scanNode!=NULL){
		edgeNode *tmp = scanNode;
		scanNode = scanNode->next;
		delete tmp;
	}

	head = NULL;
}

void printLinkList(edgeNode *head){
	edgeNode *tmp = head;
	while(tmp!=NULL){
		std::cout<<*(tmp->ed);
		tmp = tmp->next;
	}
	std::cout<<"---------------";
}

void deleteElement(edgeNode *&head, edge *t){
	edgeNode *preP = NULL;
	edgeNode *currP = head;

	while (currP!=NULL){
		if(*(currP->ed)==*t){
			removeNode(head, preP, currP);
			break;
		}
		else{
			preP = currP;
			currP =currP->next;
		}
	}
	
}

unsigned long long int size(edgeNode *head){
	unsigned long long int eleNum = 0;
	edgeNode *scanNode = head;
	while(scanNode!=NULL){
		eleNum++;
		scanNode = scanNode->next;
	}
	return eleNum;
}


//functions for triangleNode
triangleNode *createNewNode(triangle *t){
	triangleNode *tNode = new triangleNode;
	tNode->tri = t;
	tNode->next = NULL;
	return tNode;
}


void insertFront(triangleNode *&head, triangleNode *newNode){
	if(head == NULL){
		head = newNode;
		head->next = NULL;
	}
	else{
		newNode->next = head;
		head = newNode;
	}
}

void removeNode(triangleNode *&head, triangleNode *&preP, triangleNode *&currP){
	if(head == NULL) return;
	if(currP==head){//delete first node
		currP = currP->next;
		triangleNode *temp = head;
		head = currP;
		delete temp;
	}
	else{ //delete middle or last node
		triangleNode *temp = currP;
		preP->next = currP->next;
		currP = preP->next;
		delete temp;
	}
}

triangleNode *extractNode(triangleNode *&head, triangleNode *preP, triangleNode *&currP){
	if(head == NULL) return NULL;
	triangleNode *temp = NULL;
	if(currP==head){//delete first node
		currP = currP->next;
		temp = head;
		head = currP;
	}
	else{ //delete middle or last node
		temp = currP;
		preP->next = currP->next;
		currP = preP->next;
	}
	return temp;
}

void printLinkList(triangleNode *head){
	triangleNode *tmp = head;
	while(tmp!=NULL){
		std::cout<<*(tmp->tri);
		tmp = tmp->next;
	}
	std::cout<<"---------------";
}


void deleteElement(triangleNode *&head, triangle *t){
	triangleNode *preP = NULL;
	triangleNode *currP = head;

	while (currP!=NULL){
		if(*(currP->tri)==*t){
			removeNode(head, preP, currP);
			break;
		}
		else{
			preP = currP;
			currP =currP->next;
		}
	}
	
}

void removeLinkList(triangleNode *&head){
	triangleNode *scanNode = head;
	while(scanNode!=NULL){
		triangleNode *tmp = scanNode;
		scanNode = scanNode->next;
		delete tmp;
	}
	head = NULL;
}

unsigned long long int size(triangleNode *head){
	unsigned long long int eleNum = 0;
	triangleNode *scanNode = head;
	while(scanNode!=NULL){
		eleNum++;
		scanNode = scanNode->next;
	}
	return eleNum;
}

void addLinkList(triangleNode *&tempTriangleList, triangleNode *&triangleList){
	triangleNode *tail = triangleList;
	if(tail!=NULL){
		while(tail->next!=NULL)
			tail = tail->next;
		tail->next = tempTriangleList;
		tempTriangleList = NULL;
	}
	else {
		triangleList = tempTriangleList;
		tempTriangleList = NULL;
	}
}

//============================================================================
void listToTriangleIdArr(triangleNode *triangleList, unsigned long long *&triangleIdArr, unsigned &triangleNum){
	if(triangleList==NULL){
		triangleIdArr = NULL;
		triangleNum = 0;
		return;
	}
	triangleNum = size(triangleList);
	triangleIdArr = new unsigned long long[triangleNum*3];

	triangleNode *head = triangleList;
	unsigned long long index = 0;
	while(head!=NULL){
		triangleIdArr[index*3] = head->tri->p1.getId();
		triangleIdArr[index*3+1] = head->tri->p2.getId();
		triangleIdArr[index*3+2] = head->tri->p3.getId();

		index++;
		head=head->next;
	}
}

/*
////////////////////////////////////////////////////////
int main(){
	point *p1 = new point(1.0,2.0,1);
	point *p2 = new point(2.0,1.0,2);
	point *p3 = new point(2.0,2.0,3);
	point *p4 = new point(3.0,2.0,4);
	triangle *t1 = new triangle(*p1, *p2, *p3);
	triangle *t2 = new triangle(*p1, *p2, *p4);
	triangle *t3 = new triangle(*p1, *p3, *p4);
	triangle *t4= new triangle(*p2, *p3, *p4);


	triangleNode *head = new triangleNode;
	head->tri = t1;
	head->next = NULL;
//	printLinkList(head);

	triangleNode *tn1 = new triangleNode;
	tn1->tri = t2;
	tn1->next = NULL;
	insertFront(head, tn1);
//	printLinkList(head);

	triangleNode *tn2 = new triangleNode;
	tn2->tri = t3;
	tn2->next = NULL;
	insertFront(head, tn2);
//	printLinkList(head);

	triangleNode *tn3 = new triangleNode;
	tn3->tri = t4;
	tn3->next = NULL;
	insertFront(head, tn3);


	deleteElement(head, t3);

	printLinkList(head);
	removeLinkList(head);


	point *p5 = new point(1.0,2.0,1);
	point *p6 = new point(2.0,1.0,2);
	point *p7 = new point(2.0,2.0,3);
	point *p8 = new point(3.0,2.0,4);
	edge *e5 = new edge(*p5, *p6);
	edge *e6 = new edge(*p5, *p7);
	edge *e7 = new edge(*p5, *p8);
	edge *e8 = new edge(*p6, *p5);

	edgeNode *head1 = NULL;
	edgeNode *en1 = new edgeNode;
	en1->ed = e5;
	en1->next = NULL;
	insertFront(head1, en1);

	edgeNode *en2 = new edgeNode;
	en2->ed = e6;
	en2->next = NULL;
	insertFront(head1, en2);

	printLinkList(head1);
	removeLinkList(head1);


	return 0;
}
*/
