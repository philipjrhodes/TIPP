#=========================================================
#Uniform point distribuition and equal processes distribution
#=========================================================

#The first two arguments are the domain sizes of major (coarse) and minor (fine) partitions
#4x4x4x4
#(two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=4, then then we have 4x4=16 partitons

#The third argument is numper of points (10000)
#The fourth arguments is the number of processes.
#The fifth argument is the generated option (1), it has only 2 values 0 or 1. 1 means to regenerate the point dataset, 0 means reuse the dataset available.
#The sixth and seventh arguments are the data source folder (../dataSources) and triangulation results folder (../delaunayResults).
#The last argument is the folder name (source:  ../dataSources/10Kvertices, results:  ../delaunayResults/10Kvertices).
#Both source and destination folders are shared folders or non-shared folders.
#The last argument is folder name (10Kvertices). The point data located in the ../dataSources/10Kvertices/, and the delaunay results (*.tri) will be located in ../delaunayResults/10Kvertices/

#small sample with 10000 points
#./runUniform.sh 4 4 10000 4 1 ../dataSources ../delaunayResults 10Kvertices nonSharedFolder

exit


#============================test uniform point distribution=======================
#./runUniform.sh 8 8 62500000 256 1  /mnt/dataSources /mnt/dataOutput 62.5Mvertices sharedFolder
#./runUniform.sh 8 8 62500000 256 0  /mnt/dataSources /mnt/dataOutput 62.5Mvertices sharedFolder

#./runUniform.sh 16 8 125000000 256 1  /mnt/dataSources /mnt/dataOutput 125Mvertices sharedFolder
#./runUniform.sh 16 8 250000000 256 0  /mnt/dataSources /mnt/dataOutput 125Mvertices sharedFolder

#./runUniform.sh 16 8 250000000 256 1  /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#./runUniform.sh 16 8 250000000 256 0  /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder

#./runUniform.sh 16 8 500000000 256 1  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#./runUniform.sh 16 8 500000000 256 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder

exit

#========================test scalability========================================
#250 million points
#./runUniform.sh 16 8 250000000 16 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 250000000 32 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 250000000 64 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 250000000 128 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 250000000 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 250000000 512 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 250000000 1024 0 /mnt/dataSources /mnt/dataOutput 250Mvertices sharedFolder
#/home/cc/freeCluster

#============================================================================
#500 million points
#./runUniform.sh 16 8 500000000 16 1  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 500000000 32 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 500000000 64 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 500000000 128 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 500000000 256 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 500000000 512 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 8 500000000 1024 0  /mnt/dataSources /mnt/dataOutput 500Mvertices sharedFolder
#/home/cc/freeCluster

#=============================================================================
#1 billion points
#./runUniform.sh 16 16 1000000000 16 1  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 1000000000 32 0  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 1000000000 64 0  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 1000000000 128 0  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 1000000000 256 0  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 1000000000 512 0  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 1000000000 1024 0  /mnt/dataSources /mnt/dataOutput 1Bvertices sharedFolder
#/home/cc/freeCluster

#=============================================================================
#2 billion points
#./runUniform.sh 16 16 2000000000 32 1  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 2000000000 32 0  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 2000000000 64 0  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 2000000000 128 0  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 2000000000 256 0  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 2000000000 512 0  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder
#/home/cc/freeCluster
#./runUniform.sh 16 16 2000000000 1024 0  /mnt/dataSources /mnt/dataOutput 2Bvertices sharedFolder


