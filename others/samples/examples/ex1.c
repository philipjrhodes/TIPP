/*gcc ex1.c -o ex1
*/
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

main(){
	double area, pi, x;
	int i, n = 20;

	area = 0.0;

	#pragma omp parallel for private(x)
	for (i = 0; i < n; i++) {
		x = (i+0.5)/n;
//		#pragma omp critical
		area += 4.0/(1.0 + x*x);
	}
	pi = area / n;
	printf("Area = %.2f\n", area);
}
