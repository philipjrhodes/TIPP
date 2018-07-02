#include <stdio.h>

int main( )
{

	int i, n;
	char mpicmd[128];

	n = 32;

	for( i=n; i>0; i/=2 ){

		printf("Now launching %d task MPI job\n", i);
		sprintf(mpicmd, "mpirun -n %d ./parallel", i);
		system(mpicmd);

	}
}
