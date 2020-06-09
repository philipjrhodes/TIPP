#=====================================================================
#Non-Uniform point distribuition and adaptive processes distribution
#=====================================================================

#./runNonUniform.sh 4 4 10000 1 4 1 "../dataSources" "../../delaunayResults"  "10Kvertices"
./runNonUniform.sh 16 16 62500000 0 256 1 /mnt/share/dataSources /mnt/share/dataOutput 62.5Mvertices

#The first two arguments are the domain sizes of major (coarse) and minor (fine) partitions
#4x4x4x4
#(two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=4, then then we have 4x4=16 partitons.

#The third argument is the number of points in domain.

#The fourth argument is process distribution style, 0 means equally process distribution to all sub masters, 1 means DPA. The process assigning depending upon number of points in coarse (major) partitions.
#The fifth arguments is the number of processes.
#The sixth argument is the generated option (1), it has only 2 values 0 or 1. 1 means to regenerate the point dataset, 0 means reuse the dataset available.
#The seventh and eighth arguments are the data source folder (../dataSources) and triangulation results folder (../delaunayResults).
#Both source and destination folders are shared folders. All processes can access them.

#The last argument is folder name (10Kvertices). The point data located in the ../dataSources/10Kvertices/, and the delaunay results (*.tri) will be located in ../delaunayResults/10Kvertices/

exit

#=======================================================================================
#Equal process distribution (without DPA)
#=======================================================================================
./runNonUniform.sh 16 16 62500000 0 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
./runNonUniform.sh 16 16 62500000 0 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
./runNonUniform.sh 16 16 62500000 0 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices

./runNonUniform.sh 16 16 125000000 0 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices
./runNonUniform.sh 16 16 125000000 0 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices
./runNonUniform.sh 16 16 125000000 0 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices

./runNonUniform.sh 32 16 250000000 0 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices
./runNonUniform.sh 32 16 250000000 0 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices
./runNonUniform.sh 32 16 250000000 0 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices

./runNonUniform.sh 32 16 500000000 0 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 16 500000000 0 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 16 500000000 0 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices

./runNonUniform.sh 32 32 500000000 0 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 32 500000000 0 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 32 500000000 0 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices


#=======================================================================================
#Dynamic Process Assignment (DPA)
#=======================================================================================
./runNonUniform.sh 16 16 62500000 1 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
./runNonUniform.sh 16 16 62500000 1 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices
./runNonUniform.sh 16 16 62500000 1 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices

./runNonUniform.sh 16 16 125000000 1 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices
./runNonUniform.sh 16 16 125000000 1 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices
./runNonUniform.sh 16 16 125000000 1 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices

./runNonUniform.sh 32 16 250000000 1 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices
./runNonUniform.sh 32 16 250000000 1 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices
./runNonUniform.sh 32 16 250000000 1 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices

./runNonUniform.sh 32 16 500000000 1 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 16 500000000 1 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 16 500000000 1 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices

./runNonUniform.sh 32 32 500000000 1 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 32 500000000 1 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices
./runNonUniform.sh 32 32 500000000 1 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices

