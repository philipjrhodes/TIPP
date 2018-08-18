#include "TIPPList.hpp"
#include <iostream>
#include "triangle.h"
#include "point.h"
#include "edge.h"

int main(){

	point *p1 = new point(1.0,2.0,1);
	point *p2 = new point(2.0,1.0,2);
	point *p3 = new point(2.0,2.0,3);
	point *p4 = new point(3.0,2.0,4);
	
	triangle *t1 = new triangle(*p1, *p2, *p3);
	triangle *t2 = new triangle(*p1, *p2, *p4);
	triangle *t3 = new triangle(*p1, *p3, *p4);
	triangle *t4=  new triangle(*p2, *p3, *p4);

	std::cout << "main: sizeof(triangle) = " << sizeof(triangle) << std::endl;
	std::cout << "main: sizeof(Node<triangle>) = " << sizeof(TIPPList<triangle>::Node<triangle>) << std::endl;


    TIPPList<triangle> *l = new TIPPList<triangle>();

    l->insertFront(*t1);
    l->insertFront(*t2);
    l->insertFront(*t3);
    l->insertFront(*t4);
    
	l->printList();
    
    l->insertFront(*t1);
    l->insertFront(*t2);
    l->insertFront(*t3);
    l->insertFront(*t4);

    l->printList();

    l->deleteElement(*t1);
    l->deleteElement(*t2);
    l->deleteElement(*t3);
    l->deleteElement(*t4);
    
    l->printList();
    
    l->deleteElement(*t1);
    l->deleteElement(*t2);
    l->deleteElement(*t3);
    l->deleteElement(*t4);
    
    l->printList();


    delete t1;
    delete t2;
    delete t3;
    delete t4;

    delete p1;
    delete p2;    
    delete p3;
    delete p4;

      
    delete l;
    std::cerr << "Done." << std::endl;
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
