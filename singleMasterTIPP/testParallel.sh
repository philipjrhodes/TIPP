#The first argument is the number of partitions on both sizes (4) (two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....). #If we have domain size=8, then then we have 8x8=64 partitons.

#The second argument is number of points in the domain

#The third arguments is the number of processes.

#The fourth arguments (1 or 0) meaning generate point data or not.

#The fifth, sixth, seventh arguments are source path (contain point files), destination path (contain triangulation results triangleIds.tri), and folderName. (source: ../dataSources/10Kvertices, and results: ../dataSources/10Kvertices)

#The eighth argument is the option1 --> (uniform/nonUniform)

#The nineth argument is the option2 --> (sharedFolder/nonSharedFolder)


#domain size = 8; 10000 points; 4 processes; 1 --> generate new data points, 0 --> don't generate data points; 
#input data folder = 10Kvertices; triangulation result folder = 10Kvertices
#option1 = uniform/nonUniform; option2 = sharedFolder/nonSharedFolder

#sample
./runParallel.sh 8 10000 4 1 ../dataSources ../dataSources 10Kvertices uniform sharedFolder
#./runParallel.sh 8 10000 4 1 ../dataSources ../dataSources 10Kvertices nonUniform nonSharedFolder
#./runParallel.sh 8 10000 4 1 ../dataSources ../dataSources 10Kvertices uniform nonSharedFolderi

exit


#10 compute nodes, 9 OSDs (data Ceph nodes)
#options: Uniform, nonSharedFolder
#For Uniform dataset
#=============================================================================
#125 million points
#./runParallel.sh 32 62500000 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices uniform nonSharedFolder
#./runParallel.sh 32 62500000 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices uniform nonSharedFolder

#=============================================================================
#125 million points
#./runParallel.sh 32 125000000 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices uniform nonSharedFolder
#./runParallel.sh 32 125000000 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices uniform nonSharedFolder

#=============================================================================
#250 million points
#./runParallel.sh 64 250000000 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
#./runParallel.sh 64 250000000 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder

#=============================================================================
#500 million points
#./runParallel.sh 64 500000000 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
#./runParallel.sh 64 500000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder

#exit


# 10 compute nodes, 9 OSDs (data Ceph nodes)
#options: nonUniform, nonSharedFolder
#For nonUniform dataset
#=============================================================================
#125 million points
#./runParallel.sh 64 62500000 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices nonUniform nonSharedFolder
#./runParallel.sh 64 62500000 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices nonUniform nonSharedFolder

#=============================================================================
#125 million points
#./runParallel.sh 64 125000000 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices nonUniform nonSharedFolder
#./runParallel.sh 64 125000000 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices nonUniform nonSharedFolder

#=============================================================================
#250 million points
#./runParallel.sh 128 250000000 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices nonUniform nonSharedFolder
#./runParallel.sh 128 250000000 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices nonUniform nonSharedFolder

#=============================================================================
#500 million points
#./runParallel.sh 128 500000000 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices nonUniform nonSharedFolder
#./runParallel.sh 128 500000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices nonUniform nonSharedFolder




# 10 compute nodes, 9 OSDs (data Ceph nodes)
#options: nonUniform, sharedFolder
#For nonUniform dataset
#=============================================================================
#125 million points
#./runParallel.sh 64 62500000 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices nonUniform sharedFolder
#./runParallel.sh 64 62500000 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices nonUniform sharedFolder

#=============================================================================
#125 million points
#./runParallel.sh 64 125000000 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices nonUniform sharedFolder
#./runParallel.sh 64 125000000 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices nonUniform sharedFolder

#=============================================================================
#250 million points
#./runParallel.sh 128 250000000 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices nonUniform sharedFolder
#./runParallel.sh 128 250000000 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices nonUniform sharedFolder

#=============================================================================
#500 million points
#./runParallel.sh 128 500000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices nonUniform sharedFolder
#./runParallel.sh 128 500000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices nonUniform sharedFolder


#exit


#10 compute nodes, 9 OSDs (data Ceph nodes)
#options: uniform nonSharedFolder
#For nonUniform dataset
#=============================================================================
#250 million points
#./runParallel.sh  64 250000000 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
#./runParallel.sh  64 250000000 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
#=============================================================================
#500 million points
#./runParallel.sh 128 1000000000 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
#./runParallel.sh 128 1000000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder

#=============================================================================
#1 billion points
#./runParallel.sh 128 500000000 256 1 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
#./runParallel.sh 128 500000000 256 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
#=============================================================================
#2 billion points
#./runParallel.sh 256 2000000000 256 1 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
#./runParallel.sh 256 2000000000 256 0 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder




#10 compute nodes, 9 OSDs (data Ceph nodes)
#options: nonUniform sharedFolder
#For nonUniform dataset
#=============================================================================
#125 million points
#./runParallel.sh 128 62500000 256 1 /mnt/dataSources /mnt/dataOutput 62.5Mvertices nonUniform sharedFolder
#./runParallel.sh 128 62500000 256 0 /mnt/dataSources /mnt/dataOutput 62.5Mvertices nonUniform sharedFolder

#=============================================================================
#125 million points
#./runParallel.sh 512 125000000 256 1 /mnt/dataSources /mnt/dataOutput 125Mvertices nonUniform sharedFolder
#./runParallel.sh 512 125000000 256 0 /mnt/dataSources /mnt/dataOutput 125Mvertices nonUniform sharedFolder

#=============================================================================
#250 million points
#./runParallel.sh 512 250000000 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices nonUniform sharedFolder
#./runParallel.sh 512 250000000 256 1 /mnt/dataSources /mnt/dataOutput 250Mvertices nonUniform sharedFolder

#=============================================================================
#500 million points
#./runParallel.sh 512 500000000 256 1 /mnt/dataSources /mnt/dataOutput 500Mvertices nonUniform sharedFolder
#./runParallel.sh 512 500000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices nonUniform sharedFolder




#=============================== test speedup =======================================
#./runParallel.sh 8 500000 256 1 /mnt/share/dataSources /mnt/share/dataSources 500Kvertices uniform nonSharedFolder
#./runParallel.sh 8 1000000 256 0 /mnt/share/dataSources /mnt/share/dataSources 500Kvertices uniform nonSharedFolder

#./runParallel.sh 16 5000000 256 1 /mnt/share/dataSources /mnt/share/dataSources 5Mvertices uniform nonSharedFolder
#./runParallel.sh 16 5000000 256 0 /mnt/share/dataSources /mnt/share/dataSources 5Mvertices uniform nonSharedFolder

#./runParallel.sh 32 50000000 256 1 /mnt/share/dataSources /mnt/share/dataSources 50Mvertices uniform nonSharedFolder
#./runParallel.sh 32 50000000 256 0 /mnt/share/dataSources /mnt/share/dataSources 50Mvertices uniform nonSharedFolder

#./runParallel.sh 64 500000000 256 1 /mnt/share/dataSources /mnt/share/dataSources 500Mvertices uniform nonSharedFolder
#./runParallel.sh 64 500000000 256 0 /mnt/share/dataSources /mnt/share/dataSources 500Mvertices uniform nonSharedFolder


#========================================test scalability ==========================================
#10 compute nodes, 9 OSDs (data Ceph nodes)
#options: Uniform, nonSharedFolder
#For Uniform dataset

#250 million points
#./runParallel.sh 64 250000000 16 1 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 250000000 32 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 250000000 64 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 250000000 128 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 250000000 256 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 250000000 512 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 250000000 1024 0 /mnt/dataSources /mnt/dataOutput 250Mvertices uniform nonSharedFolder

#=============================================================================
#500 million points
#./runParallel.sh 64 500000000 16 1 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 500000000 32 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 500000000 64 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 500000000 128 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 500000000 256 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 500000000 512 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 64 500000000 1024 0 /mnt/dataSources /mnt/dataOutput 500Mvertices uniform nonSharedFolder

#=============================================================================
#1 billion points
#./runParallel.sh 128 1000000000 16 1 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 1000000000 32 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 1000000000 64 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 1000000000 128 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 1000000000 256 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 1000000000 512 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 1000000000 1024 0 /mnt/dataSources /mnt/dataOutput 1Bvertices uniform nonSharedFolder
/home/cc/freeCluster

#=============================================================================
#2 billion points
./runParallel.sh 128 2000000000 32 1 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 2000000000 64 0 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 2000000000 128 0 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 2000000000 256 0 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 2000000000 512 0 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
/home/cc/freeCluster
./runParallel.sh 128 2000000000 1024 0 /mnt/dataSources /mnt/dataOutput 2Bvertices uniform nonSharedFolder
/home/cc/freeCluster
