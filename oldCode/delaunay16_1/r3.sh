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


#mpic++ -std=gnu++11 linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp delaunayMPI.cpp coarsePartition.cpp delaunay2DMain.cpp -o delaunay2DMain


#The last parameter is the domain size

mpiexec -n 17 -f machinefile ./TIPP "../dataSources/10Kvertices/" 1
#mpiexec -n 16 -f machinefile ./delaunay2DMain "../dataSources/15Kvertices/" 1
#mpiexec -n 22 -f machinefile ./delaunay2DMain "../dataSources/20Kvertices/" 1


#4 means domainSize
#if domainSize=1, then the domain is square between 0,0 - 1,1
#if domainSize=3, then the domain is square between 0,0 - 3,3
#./domain "../dataSources/4.5Bvertices/" 4 252
 

#./domain "../dataSources/500Mvertices/" 468
#gprof ./domain > gprofResult32x32.txt
#gprof ./domain > gprofResult64x64.txt
#gprof ./domain > gprofResult128x128.txt



#468 means the number of available cores in hpcwoods.olemiss.edu
#./domain "../dataSources/15Mvertices/" 468
#./domain "../dataSources/30Mvertices/" 468
#./domain "../dataSources/60Mvertices/" 360
#./domain "../dataSources/120Mvertices/" 360
#./domain "../dataSources/240Mvertices/" 360
#./domain "../dataSources/480Mvertices/" 360



#sudo pkill -9 domain
