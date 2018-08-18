
#ifndef TIPPLIST_H
#define TIPPLIST_H

#include <iostream>
#include <assert.h>
#include <stdlib.h>




template <class T>
class TIPPList{

    public: 
    
        template <class V>
        class Node{
            public: 
                Node(V v, Node<V> *n = NULL){
                    
                    data = v;
                    next = n;
                }
                
                ~Node(){
                
                    // data destructor is called automatically.
                                        
                }
         

                V data;
                Node<V> * next = NULL;
        };
  
    
    
    
        TIPPList() { 
            head = NULL; 
            count = 0;
        }
  
        
        void insertFront(const T & e){

            Node<T> *n = new Node<T>(e, head);
            head = n;
            count++;
        }
        
        /** Remove the first occurence of e from the 
            list and delete it.
        */
        void deleteElement(const T & e){
        
            Node<T> *prev = NULL;
            Node<T> *cur = head;

            while (cur != NULL){
                if(cur->data == e){                    
                    removeNode(prev, cur);
                    return;
                } else {
                    prev = cur;
                    cur = cur->next;
                }
            }
            count--;
        }
        
        
        
        /** Remove the Node pointed to by cur from the list, using
            its predecessor *prev, if necessary. The removed node is
            deleted.
        */
        inline void removeNode(Node<T> *prev, Node<T> *cur){
        
            Node<T> * n = extractNode(prev, cur);
            delete n;
            count--;
        }

        /** Remove the Node pointed to by cur from the list, using
            its predecessor *prev, if necessary. Return a pointer to
            the removed Node.
        */
        Node<T> * extractNode(Node<T> *prev, Node<T> *cur){
            
            assert(cur != NULL);            
            
            if(cur == head){//delete first node
                head = cur->next;
            } else { //delete middle or last node
                prev->next = cur->next;
            }
            
            count--;
            return cur;
        }


          




        void printList(){
            Node<T> *tmp = head;
            while(tmp!=NULL){
                std::cout << tmp->data;  // TODO
                tmp = tmp->next;
            }
            std::cout<<"---------------" << std::endl;   
        }
 
 
 
       
        unsigned long long int size(){
        
            return count;
        }
        
        void addLinkList(TIPPList<T> &l);

       
        virtual ~TIPPList(){
            
            Node<T> *c = head;
            Node<T> *p = NULL;
            
            while (c != NULL){
                p = c;
                c = c->next;
                delete p;
            }
            
           head = NULL;
           count = 0;     
        }
        

    protected:
        Node<T> * head;
        long long int count = 0;
    
    private:
    

    
};

#endif
