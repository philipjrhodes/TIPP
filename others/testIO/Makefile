


readMPI: readMPI.c
	mpicc readMPI.c -O3 -o readMPI
	./copyExec readMPI

run: readMPI
	mpiexec -n 1024 -f machinefile ./readMPI 10G 
