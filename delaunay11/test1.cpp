#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "point.h"

//check whether a point stay inside a partititon
bool insidePartion(point *pointCoorArr, int size, double domainSize, int xPartNum, int yPartNum, int partId){
        double xPartSize = domainSize/xPartNum;
        double yPartSize = domainSize/yPartNum;
        double lowPointX = (partId%yPartNum)*xPartSize;
        double lowPointY = (partId/yPartNum)*yPartSize;
        double highPointX = lowPointX + xPartSize;
        double highPointY = lowPointY + yPartSize;

        for(int i=0; i<size; i++){
                double x = pointCoorArr[i].getX();
                double y = pointCoorArr[i].getY();
                if((x<lowPointX)||(y<lowPointY)||(x>highPointX)||(y>highPointY)){
                        std::cout<<x<<" "<<y<<" "<<lowPointX<<" "<<lowPointY<<" "<<highPointX<<" "<<highPointY<<"\n";
                        //return false;
                }
        }
        return true;
}

int main(){
		std::string path = "../dataSources/100vertices/delaunayResults/pointCoorPart.ver";
        FILE *f = fopen(path.c_str(),"rb");
        if(!f){
                std::cout<<"Can not open file!!\n";
                exit(1);
        }
        int size = 4;
        double *pointCoorArr = new double[size*2];
        unsigned int offset =  0;
        fseek(f, offset*2*sizeof(double), SEEK_SET);
        fread(pointCoorArr, sizeof(double), size*2, f);
        fclose(f);
		std::cout.precision(16);
        double domainSize = 1;
        int xPartNum = 4;
        int yPartNum = 4;
        int partId = 0;

		for(int i=0; i<size; i++)
			std::cout<<pointCoorArr[i*2]<<" "<<pointCoorArr[i*2+1]<<"\n";

//        if(insidePartion(pointCoorArr, size, domainSize, xPartNum, yPartNum, partId))
//              std::cout<<"partition: "<<partId<<" is good!!\n";

        delete [] pointCoorArr;
        return 0;
}

