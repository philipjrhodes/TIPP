#include <iostream>
#include <stdlib.h>
int main(){
	unsigned long long N = 10000000000000000;
//	unsigned long long N = 1000;
	try {
		double* c = new double[N];
		if(!c) 		std::cout<<"Memory overflow!!!!!!\n";
	} catch (std::bad_alloc&) {
	  // Handle error
		std::cout<<"Memory overflow\n";
		exit(1);
	}
	std::cout<<"Allocation is ok!!!!\n";
}
