#include <stdio.h>

int main()
{
double *arr;
int i;
//FILE *f = fopen("pointPart.ver", "r");
FILE *f = fopen("../dataSources/100vertices/pointPartitions/pointPart.ver", "r");
fseek(f, 0L, SEEK_END);
int size = ftell(f);
arr = (double*) malloc(size);
fseek(f, 0, SEEK_SET); 
int N = size/sizeof(double);
fread(arr, N, sizeof(double), f);
fclose(f);

for(i=0; i<N; i++) printf("%.1f ", arr[i]);
   return 0;
}
