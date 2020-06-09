#ssh r2449@hpcwoods.olemiss.edu,
#ssh r2449@maple, Changeme12!@

#!/bin/sh
#PBS -N delaunayMPI
#PBS -l select=256:ncpus=1:mem=3gb -l place=free

#module load pbspro
#module load mvapich2

#cd ${PBS_O_WORKDIR}

#hostname
#qstat -f $PBS_JOBID
#mpiexec ./delaunayMPI "../dataSources/10Kvertices/" 1
#The last argument is the domainSize



#The last parameter is the domain size
#dataName, domainSize, number of nodes in cluster
mpiexec -n 16 -f machinefile ./TIPP "15Kvertices" 1 3

#mpiexec -n 16 -f machinefile ./delaunay2DMain "../dataSources/15Kvertices/" 1
#mpiexec -n 22 -f machinefile ./delaunay2DMain "../dataSources/20Kvertices/" 1

#./domain "../dataSources/500Mvertices/" 468
#gprof ./domain > gprofResult32x32.txt
#gprof ./domain > gprofResult64x64.txt
#gprof ./domain > gprofResult128x128.txt



#sudo pkill -9 domain

#mpic++ -std=gnu++11 linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp delaunayMPI.cpp coarsePartition.cpp delaunay2DMain.cpp -o delaunay2DMain

