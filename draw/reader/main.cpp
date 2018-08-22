#include <iostream>
#include <stdlib.h>
#include "PDFCanvas.h"
#include "TIPPList.hpp"
#include "triangle.h"
#include "point.h"
#include "edge.h"
#include "ListReader.h"

using namespace std;


void usage(){
    cout << "Usage: rd filename.tri [filename.ver]" << endl;
    exit(1);
}


int main(int argc, char * argv[]){
    
    ListReader r;
    
    if (argc == 2) {
    	r = ListReader(argv[1]);
    } else if (argc == 3) {
        r = ListReader(argv[1], argv[2]);
    } else
        usage();


    //"drawData/initTrianglesDomain/domainTriangles.tri"
    //ListReader r(argv[1], argv[2]);
    
    r.readTriangles();
    
//     TIPPList<triangle> tlist = r.getTriangleList();
//     
//     if(tlist.size() == 0){
//         
//         cout << "tlist is empty." << endl;
//     }
    
    int numElements=0;
    triangle * tarr = r.getTriangleArray(numElements);
    
    if(NULL == tarr){
        
        cout << "triangle array is empty." << endl;
    }
    
    std::vector<triangle> tvec (tarr, tarr + numElements); // make a vector from the array.
    
    cout << "triangle vector has length " << tvec.size() << endl;
     cout << "triangle[0] " << tvec[0] << endl;
    
    Canvas *c = new PDFCanvas(argv[1]);
 
 	
 	c->setMapping(tvec);
 	//c->setMapping(0,0,1,1);
 	
 	c->setStrokeColor(1, 0, 0);
 	c->setFillColor(0.9, 0.9, 1);
 	c->setStrokeWidth(2.0);
 	
//  	c->drawRect(200,600, 100,100);
//  	c->drawRect(200,500, 100,100);
//  	c->drawRect(100,500, 100,100);
//  	c->drawRect(100,600, 100,100);

	c->setStrokeWidth(0.01);
 	
 	c->enableFill();
//  	c->drawCircle(300,400, 100);
 	
 	c->setStrokeColor(0, 0, 0);
 	c->disableFill();

  	c->drawTriangles(tvec);	
    
	c->saveToFile("reader.pdf");
	
	delete c;

    
    
    std::cerr << "main Done." << std::endl;
    
// 
//  triangleNode *head = new triangleNode;
//  head->tri = t1;
//  head->next = NULL;
// //   printLinkList(head);
// 
//  triangleNode *tn1 = new triangleNode;
//  tn1->tri = t2;
//  tn1->next = NULL;
//  insertFront(head, tn1);
// //   printLinkList(head);
// 
//  triangleNode *tn2 = new triangleNode;
//  tn2->tri = t3;
//  tn2->next = NULL;
//  insertFront(head, tn2);
// //   printLinkList(head);
// 
//  triangleNode *tn3 = new triangleNode;
//  tn3->tri = t4;
//  tn3->next = NULL;
//  insertFront(head, tn3);
// 
// 
//  deleteElement(head, t3);
// 
//  printLinkList(head);
//  removeLinkList(head);
// 
// 
//  point *p5 = new point(1.0,2.0,1);
//  point *p6 = new point(2.0,1.0,2);
//  point *p7 = new point(2.0,2.0,3);
//  point *p8 = new point(3.0,2.0,4);
//  edge *e5 = new edge(*p5, *p6);
//  edge *e6 = new edge(*p5, *p7);
//  edge *e7 = new edge(*p5, *p8);
//  edge *e8 = new edge(*p6, *p5);
// 
//  edgeNode *head1 = NULL;
//  edgeNode *en1 = new edgeNode;
//  en1->ed = e5;
//  en1->next = NULL;
//  insertFront(head1, en1);
// 
//  edgeNode *en2 = new edgeNode;
//  en2->ed = e6;
//  en2->next = NULL;
//  insertFront(head1, en2);
// 
//  printLinkList(head1);
//  removeLinkList(head1);


    return 0;
}


int test1(){

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
    std::cerr << "t1 Done." << std::endl;
    return 0;
}

