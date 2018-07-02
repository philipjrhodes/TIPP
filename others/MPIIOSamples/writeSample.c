#include <stdio.h>
#define Size 40
int main()
{
int arr1[Size], arr2[Size];
int i;
for(i=0; i<Size; i++) arr1[i]=i;
FILE *f = fopen("testfile", "w");
fwrite(arr1, Size, sizeof(int), f);
fclose(f);

f = fopen("testfile", "r");
fread(arr2, Size, sizeof(int), f);
fclose(f);

for(i=0; i<Size; i++) printf("%d ", arr2[i]);
   return 0;
}
