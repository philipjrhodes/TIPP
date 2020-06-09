#The first argument is the number of partitions on both sizes (4) (two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....). #If we have domain size=8, then then we have 8x8=64 partitons.

#The second argument is number of points in the domain

#The third arguments (1 or 0) meaning generate point data or not.

#The fourth, fifth, sixth arguments are source path (contain point files), destination path (contain triangulation results triangleIds.tri), and folderName. (source: ../dataSources/10Kvertices, and results: ../dataSources/10Kvertices)

#The eighth argument is an option --> (uniform/nonUniform)


#domain size = 8; 10000 points; 4 processes; 1 --> generate new data points, 0 --> don't generate data points; 
#input data folder = 10Kvertices; triangulation result folder = 10Kvertices
#option = uniform/nonUnifom

#sample
./runSerial.sh 8 10000 1 ../dataSources ../dataSources 10Kvertices uniform
#./runSerial.sh 8 10000 1 ../dataSources ../dataSources 10Kvertices nonUniform
#./runSerial.sh 16 500000 1 ../dataSources ../dataSources 500Kvertices nonUniform

exit

#Run serial on one Chameleon machine
#options: unifom
#For nonUnifom dataset
#=============================================================================
#1 million points
./runSerial.sh 8 1000000 1 ../dataSources ../dataOutput 1Mvertices uniform
./runSerial.sh 8 1000000 0 ../dataSources ../dataOutput 1Mvertices uniform
#=============================================================================
#10 million points
./runSerial.sh 16 10000000 1 ../dataSources ../dataOutput 10Mvertices uniform
./runSerial.sh 16 10000000 0 ../dataSources ../dataOutput 10Mvertices uniform

#=============================================================================
#100 million points
./runSerial.sh 32 100000000 1 ../dataSources ../dataOutput 100Mvertices uniform
./runSerial.sh 32 100000000 0 ../dataSources ../dataOutput 100Mvertices uniform
#=============================================================================
#1 billion points
./runSerial.sh 64 1000000000 1 ../dataSources ../dataOutput 1Bvertices uniform
./runSerial.sh 64 1000000000 0 ../dataSources ../dataOutput 1Bvertices uniform




#Run serial on one Chameleon machine
#options: unifom
#For nonUnifom dataset
#=============================================================================
#1 million points
./runSerial.sh 8 1000000 1 ../dataSources ../dataOutput 1Mvertices nonUniform
./runSerial.sh 8 1000000 0 ../dataSources ../dataOutput 1Mvertices nonUniform
#=============================================================================
#10 million points
./runSerial.sh 16 10000000 1 ../dataSources ../dataOutput 10Mvertices nonUniform
./runSerial.sh 16 10000000 0 ../dataSources ../dataOutput 10Mvertices nonUniform

#=============================================================================
#100 million points
./runSerial.sh 32 100000000 1 ../dataSources ../dataOutput 100Mvertices nonUniform
./runSerial.sh 32 100000000 0 ../dataSources ../dataOutput 100Mvertices nonUniform
#=============================================================================
#1 billion points
./runSerial.sh 64 1000000000 1 ../dataSources ../dataOutput 1Bvertices nonUniform
./runSerial.sh 64 1000000000 0 ../dataSources ../dataOutput 1Bvertices nonUniform


