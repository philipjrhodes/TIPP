
#ifndef TIPPLIST_H
#define TIPPLIST_H

template <class T>
class Node{
	public: 
        Node(){
        
            next = NULL;
        }
 
        int equals(T &e){

            return e == data;
        }
        
    protected:
        T data;
        Node<T> * next = NULL;
        
//    friend class TIPPList;
};


template <class T>
class TIPPList{

    public: 
        TIPPList() { head = NULL; }
        
        void insertFront(const T &n);
        
        void removeNode(Node<T> *prev, Node<T> *cur);

        Node<T> * extractNode(Node<T> *prev, Node<T> *cur);
        
        void printLinkList();
        
        void deleteElement(const T & e);
       
        unsigned long long int size();
        
        void addLinkList(TIPPList<T> &l);

       
        ~TIPPList();
        
    protected:
        Node<T> * head;
        
    
    private:
    
};

#endif
