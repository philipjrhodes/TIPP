#include <iostream>
#include <math.h>

int main(){
double scale = 10e6;
double x1= 0.750506284879992047;
double y1=0.759124104640562236;
double x2= 0.75252923951719819; 
double y2=0.758922676331291624;
double x3= 0.751517762198595118; 
double y3=0.75902339048592693;

x1 = x1*scale;
y1 = y1*scale;
x2 = x2*scale;
y2 = y2*scale;
x3 = x3*scale;
y3 = y3*scale;

double epsilon = 1e-25;
std::cout.precision(18);
double abs = fabs((x1-x2)*(y2-y3)-(y1-y2)*(x2-x3));
if(abs<epsilon)
	std::cout<<"it is not a triangle\n";
else{
	 std::cout<<"it is a triangle "<<std::endl;
}

std::cout<<"triangle area: "<<fabs((x1-x2)*(y2-y3)-(y1-y2)*(x2-x3))<<std::endl;
return 0;

}
