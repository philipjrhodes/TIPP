#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "common.h"

int main(){
	int N = 4*4;
	FILE *f;
	std::string fileStr = "../dataSources/100vertices/delaunayResults/pointPart";
	for(int i=0; i<N; i++){
		fileStr = generateFileName(i, fileStr, N);
		f = fopen(fileStr.c_str(), "rb");
		if(!f) std::cout<<"********not exist file "<<fileStr<<"\n";
		fclose(f);
		fileStr = "../dataSources/100vertices/delaunayResults/pointPart";
	}
    return 0;
}

