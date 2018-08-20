#include <iostream>
#include "TIPPList.h"

//g++ common.cpp point.cpp edge.cpp triangle.cpp linkList.cpp -o ll

//functions for edgeNode
// edgeNode *createNewNode(edge *e){
// 	edgeNode *eNode = new edgeNode;
// 	eNode->ed = e;
// 	eNode->next = NULL;
// 	return eNode;
// }


//deprecated
// void TIPPList<T>::insertFront(Node<T> &newNode){
// 
// 	if(head == NULL){
// 		head = &newNode;
// 		head->next = NULL;
// 	}
// 	else{
// 		newNode.next = head;
// 		head = &newNode;
// 	}
// }


// void TIPPList<T>::insertFront(const T & e){
// 
//     Node<T> *n = new Node<T>();
//     n->data = e;
//     n->next = head;
//     head = n;
// }


//deprecated
// void TIPPList::insertFront(Node<T> * newNode){
// 
// 	if(head == NULL){
// 		head = newNode;
// 		head->next = NULL;
// 	}
// 	else{
// 		newNode->next = head;
// 		head = newNode;
// 	}
// }
// 
// 
// void TIPPList::insertFront(triangle &t){
// 
//     insertFront(new triangleNode(t));
// }
// 
// void TIPPList::insertFront(edge &e){
// 
//     insertFront(new edgeNode(e));
// }




void TIPPList::removeNode( Node<T> *preP, Node<T> *currP){

    Node<T> * n = extractNode(preP, currP);
    delete n;
}

//edgeNode *extractNode(edgeNode *&head, edgeNode *preP, edgeNode *&currP){
Node<T> * TIPPList::extractNode(Node<T> *prev, Node<T> *cur){

	Node<T> *temp = NULL;
	if(cur == head){//delete first node
		cur = cur->next;
		temp = head;
		head = cur;
	}
	else{ //delete middle or last node
		temp = cur;
		prev->next = cur->next;
		cur = prev->next;
	}
	return temp;
}

TIPPList::~TIPPList(){

	Node<T> *scanNode = head;
	while(scanNode!=NULL){
		Node *tmp = scanNode;
		scanNode = scanNode->next;
		delete tmp;
	}

	head = NULL;
}

void TIPPList::printLinkList(){

	Node<T> *tmp = head;
	while(tmp!=NULL){
		std::cout << *(tmp->data);  // TODO
		tmp = tmp->next;
	}
	std::cout<<"---------------";
}

// void TIPPList::deleteElement(edge &e){
// 	edgeNode *preP = NULL;
// 	edgeNode *currP = (edgeNode *) head;
// 
// 	while (currP!=NULL){
// 		if(currP->equals(e)){
// 			removeNode(preP, currP);
// 			break;
// 		}
// 		else{
// 			preP = currP;
// 			currP = (edgeNode *)(currP->next);
// 		}
// 	}
// }
// 
// void TIPPList::deleteElement(triangle &t){
// 	triangleNode *preP = NULL;
// 	triangleNode *currP = (triangleNode *) head;
// 
// 	while (currP!=NULL){
// 		if(currP->equals(t)){
// 			removeNode(preP, currP);
// 			break;
// 		}
// 		else{
// 			preP = currP;
// 			currP = (triangleNode *) (currP->next);
// 		}
// 	}
// 	
// }

void TIPPList::deleteElement(const T & e){
	Node<T> *preP = NULL;
	Node<T> *currP = head;

	while (currP!=NULL){
		if(currP->equals(t)){
			removeNode(preP, currP);
			break;
		}
		else{
			preP = currP;
			currP = currP->next);
		}
	}
	
}



unsigned long long int TIPPList::size(){

	unsigned long long int eleNum = 0;
	
	Node<T> *scanNode = head;
	while(scanNode!=NULL){
		eleNum++;
		scanNode = scanNode->next;
	}
	return eleNum;
}

// 
// functions for triangleNode
// triangleNode *createNewNode(triangle *t){
// 	triangleNode *tNode = new triangleNode;
// 	tNode->tri = t;
// 	tNode->next = NULL;
// 	return tNode;
// }
// 
// 
// void insertFront(triangleNode *&head, triangleNode *newNode){
// 	if(head == NULL){
// 		head = newNode;
// 		head->next = NULL;
// 	}
// 	else{
// 		newNode->next = head;
// 		head = newNode;
// 	}
// }
// 
// void removeNode(triangleNode *&head, triangleNode *&preP, triangleNode *&currP){
// 	if(head == NULL) return;
// 	if(currP==head){//delete first node
// 		currP = currP->next;
// 		triangleNode *temp = head;
// 		head = currP;
// 		delete temp;
// 	}
// 	else{ //delete middle or last node
// 		triangleNode *temp = currP;
// 		preP->next = currP->next;
// 		currP = preP->next;
// 		delete temp;
// 	}
// }
// 
// triangleNode *extractNode(triangleNode *&head, triangleNode *preP, triangleNode *&currP){
// 	if(head == NULL) return NULL;
// 	triangleNode *temp = NULL;
// 	if(currP==head){//delete first node
// 		currP = currP->next;
// 		temp = head;
// 		head = currP;
// 	}
// 	else{ //delete middle or last node
// 		temp = currP;
// 		preP->next = currP->next;
// 		currP = preP->next;
// 	}
// 	return temp;
// }
// 
// void printLinkList(triangleNode *head){
// 	triangleNode *tmp = head;
// 	while(tmp!=NULL){
// 		std::cout<<*(tmp->tri);
// 		tmp = tmp->next;
// 	}
// 	std::cout<<"---------------";
// }
// 
// 
// void deleteElement(triangleNode *&head, triangle *t){
// 	triangleNode *preP = NULL;
// 	triangleNode *currP = head;
// 
// 	while (currP!=NULL){
// 		if(*(currP->tri)==*t){
// 			removeNode(head, preP, currP);
// 			break;
// 		}
// 		else{
// 			preP = currP;
// 			currP =currP->next;
// 		}
// 	}
// 	
// }

// void removeLinkList(triangleNode *&head){
// 	triangleNode *scanNode = head;
// 	while(scanNode!=NULL){
// 		triangleNode *tmp = scanNode;
// 		scanNode = scanNode->next;
// 		delete tmp;
// 	}
// 	head = NULL;
// }

// unsigned long long int size(triangleNode *head){
// 	unsigned long long int eleNum = 0;
// 	triangleNode *scanNode = head;
// 	while(scanNode!=NULL){
// 		eleNum++;
// 		scanNode = scanNode->next;
// 	}
// 	return eleNum;
// }

/** Append the argument list to the tail of this list. The argument list
 *  will no longer refer to its elements.
 */
void TIPPList::addLinkList(TIPPList &src){
	Node *tail = head;
	Node * prev=NULL;
	
	while(tail != NULL) {
	    prev = tail;
		tail = tail->next;
	}
	
	if(prev != NULL){
		prev->next = src.head;
		src.head = NULL;
	}
	else {
		this->head = src.head;
		src.head = NULL;             
	}
	
}


int main(){

	point *p1 = new point(1.0,2.0,1);
	point *p2 = new point(2.0,1.0,2);
	point *p3 = new point(2.0,2.0,3);
	point *p4 = new point(3.0,2.0,4);
	
	triangle *t1 = new triangle(*p1, *p2, *p3);
	triangle *t2 = new triangle(*p1, *p2, *p4);
	triangle *t3 = new triangle(*p1, *p3, *p4);
	triangle *t4=  new triangle(*p2, *p3, *p4);

    TIPPList *l = new TIPPList();
    
    l->insertFront(*t1);
    l->insertFront(*t2);
    l->insertFront(*t3);
    l->insertFront(*t4);

    l->insertFront(*t1);
    l->insertFront(*t2);
    l->insertFront(*t3);
    l->insertFront(*t4);
    
    l->deleteElement(*t1);
    l->deleteElement(*t2);
    l->deleteElement(*t3);
    l->deleteElement(*t4);
    
    delete t1;
    delete t2;
    delete t3;
    delete t4;
    
    delete p1;
    delete p2;    
    delete p3;
    delete p4;
        
    delete l;



// 
// 	triangleNode *head = new triangleNode;
// 	head->tri = t1;
// 	head->next = NULL;
// //	printLinkList(head);
// 
// 	triangleNode *tn1 = new triangleNode;
// 	tn1->tri = t2;
// 	tn1->next = NULL;
// 	insertFront(head, tn1);
// //	printLinkList(head);
// 
// 	triangleNode *tn2 = new triangleNode;
// 	tn2->tri = t3;
// 	tn2->next = NULL;
// 	insertFront(head, tn2);
// //	printLinkList(head);
// 
// 	triangleNode *tn3 = new triangleNode;
// 	tn3->tri = t4;
// 	tn3->next = NULL;
// 	insertFront(head, tn3);
// 
// 
// 	deleteElement(head, t3);
// 
// 	printLinkList(head);
// 	removeLinkList(head);
// 
// 
// 	point *p5 = new point(1.0,2.0,1);
// 	point *p6 = new point(2.0,1.0,2);
// 	point *p7 = new point(2.0,2.0,3);
// 	point *p8 = new point(3.0,2.0,4);
// 	edge *e5 = new edge(*p5, *p6);
// 	edge *e6 = new edge(*p5, *p7);
// 	edge *e7 = new edge(*p5, *p8);
// 	edge *e8 = new edge(*p6, *p5);
// 
// 	edgeNode *head1 = NULL;
// 	edgeNode *en1 = new edgeNode;
// 	en1->ed = e5;
// 	en1->next = NULL;
// 	insertFront(head1, en1);
// 
// 	edgeNode *en2 = new edgeNode;
// 	en2->ed = e6;
// 	en2->next = NULL;
// 	insertFront(head1, en2);
// 
// 	printLinkList(head1);
// 	removeLinkList(head1);


	return 0;
}

