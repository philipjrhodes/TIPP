#Non-Uniform point distribuition and adaptive processes distribution

#./runNonUniform.sh 16 50 10000 1 4 1 "../dataSources" "../delaunayResults"  "10Kvertices"
./runNonUniformARP_DPA.sh 16 10000 62500000 1 256 1 /mnt/share/dataSources /mnt/share/dataOutput 62.5Mvertice

#The first argument is domain size on both sides (4, 8, 16, 64, 256, ..., default = 16 )
#The second argument is threshold number (number of points in a fine partition in a coarse partition)
#The third argument is the number of points in domain

#The fourth argument is process distribution style, 0 means equally process distribution to all sub masters, 1 means DPA. The process assigning depending upon number of points in coarse (major) partitions.
#The fifth arguments is the number of processes.
#The sixth argument is the generated option (1), it has only 2 values 0 or 1. 1 means to regenerate the point dataset, 0 means reuse the dataset available.
#The seventh and eighth arguments are the data source folder (../dataSources) and triangulation results folder (../delaunayResults).
#Both source and destination folders are shared folders. All processes can access them.

#The last argument is folder name (10Kvertices). The point data located in the ../dataSources/10Kvertices/, and the delaunay results (*.tri) will be located in ../delaunayResults/10Kvertices/

#domain size = 8, 10000 points, 1 = adaptive process distribution, 4 processes, 1 = generate new data points, data folder 10Kvertices

exit

#=================================================
#Adaptive process distribution
#=================================================

#62.5 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 10000 62500000 1 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
./runNonUniformARP_DPA.sh 16 10000  62500000 1 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
#=============================================================================
#125 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 10000 125000000 1 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices
./runNonUniformARP_DPA.sh 16 10000 125000000 1 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices
#=============================================================================
#250 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 15000 250000000 1 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices
./runNonUniformARP_DPA.sh 16 15000 250000000 1 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices
#=============================================================================
#500 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 30000 500000000 1 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniformARP_DPA.sh 16 30000 500000000 1 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices


#========================================
#Equal process distribution
#========================================

#62.5 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 10000 62500000 0 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mveritices
./runNonUniformARP_DPA.sh 16 10000  62500000 0 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
#=============================================================================
#125 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 10000 125000000 0 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices
./runNonUniformARP_DPA.sh 16 10000 125000000 0 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices
#=============================================================================
#250 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 15000 250000000 0 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices
./runNonUniformARP_DPA.sh 16 15000 250000000 0 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices
#=============================================================================
#500 million points
#/home/cc/freeCluster
./runNonUniformARP_DPA.sh 16 30000 500000000 0 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniformARP_DPA.sh 16 30000 500000000 0 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices


