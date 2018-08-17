#include "triangle.h"

#ifndef H_LINKLIST
#define H_LINKLIST

struct edgeNode{
	edge *ed;
	edgeNode *next;
	~edgeNode(){
		delete ed;
	}
};


struct triangleNode{
	triangle *tri;
	triangleNode *next;
	~triangleNode(){
		delete tri;
	}
};

edgeNode *createNewNode(edge *e);

void insertFront(edgeNode *&head, edgeNode *newNode);

void removeNode(edgeNode *&head, edgeNode *&preP, edgeNode *&currP);

edgeNode *extractNode(edgeNode *&head, edgeNode *preP, edgeNode *&currP);

void printLinkList(edgeNode *head);

void deleteElement(edgeNode *&head, edge *t);

void removeLinkList(edgeNode *&head);

unsigned long long int size(edgeNode *head);



triangleNode *createNewNode(triangle *t);

void insertFront(triangleNode *&head, triangleNode *newNode);

void removeNode(triangleNode *&head, triangleNode *&preP, triangleNode *&currP);

triangleNode *extractNode(triangleNode *&head, triangleNode *preP, triangleNode *&currP);

void printLinkList(triangleNode *head);

void deleteElement(triangleNode *&head, triangle *t);

void removeLinkList(triangleNode *&head);

unsigned long long int size(triangleNode *head);

void addLinkList(triangleNode *&tempTriangleList, triangleNode *&triangleList);

#endif
