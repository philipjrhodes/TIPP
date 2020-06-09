#!/bin/bash 

#./runNonUniform.sh 4 4 10000 1 4 1 "../dataSources" "../../delaunayResults"  "10Kvertices"

#The first two arguments are the domain sizes of major (coarse) and minor (fine) partitions
#4x4x4x4
#(two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=4, then then we have 4x4=16 partitons.

#The third argument is the number of points in domain.

#The fourth argument is process distribution style, 0 means equally process distribution to all sub masters, 1 means DPA. The process assigning depending upon number of points in coarse (major) partitions.
#The fifth arguments is the number of processes.
#The sixth argument is the generated option (1), it has obnly 2 values 0 or 1. 1 means to regenerate the point dataset, 0 means reuse the dataset available.
#The seventh and eighth arguments are the data source folder (../dataSources) and triangulation results folder (../delaunayResults).
#The last argument is folder name (10Kvertices). The point


if [ "$#" != 9 ]; then
	echo "number of arguments should be 9"
	echo "The first argument is domain size on both sides"
	echo "The second argument is threshold number (number of points in a fine partition in a coarse partition)"
	echo "The third argument is the number of points in domain"
	echo "The fourth argument is process distribution style, 0 means equally process distribution to all sub masters, 1 means adaptive distribution depending upon number opf points in coarse (major) partitions"
	echo "The fifth arguments is the number of processes"
	echo "The sixth arguments (1 or 0) meaning generate point data or not"
	echo "The seventh and eighth argument are source and destination paths"
	echo "The nineth argument is the name of data folder"
	echo "for example:"
	echo "./runNonUniform.sh 16 200 10000 1 4 1 ../dataSources ../dataSources  10Kvertices"

	exit
fi

#echo " all arguments: $@"

#not exist folder $7 or $8
if [ ! -d $7  -o  ! -d $8 ]; then
	mkdir $7
	mkdir $7/$9
	if [ "$7" != "$8" ]; then
		mkdir $8
		mkdir $8/$9
	fi
else
	if [ ! -d $7/$9 ]; then #not exist $9 in $7
		mkdir $7/$9
	else
		if [ $6 -eq 1 ]; then
			rm -rf $7/$9
			mkdir $7/$9
		fi
	fi

	if [ ! -d $8/$9 ]; then #not exist $9 in $8
		mkdir $8/$9
	else
		if [ "$7" != "$8" ]; then
			rm -rf $8/$9
			mkdir $8/$9
		fi
	fi
fi


#1. Generate data points in each partitions, this need to use rbox from QHULL.
#Important: QHULL SHOULD BE INSTALLED BEFORE RUNNING THIS GENERATING AND DELAUNAY
#We should generate points in each partition before running the Delaunay (TIPP)
#compile
#g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute

#The first four arguments are domain sizes (both sizes are the same $2 $2 (ex: 4 4), and subdomain sizes ($3 $3) 
#Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=4, then then we have 4x4=16 domain partitons, If we have sub domain size=4 --> we have 4x4=16 sub partitions (a partition has 16 sub partitions)
#The fifth argument is numper of points ($4) in the domain (ex: 10000),
#The sixth arguments is source path (ex:  ../dataSources/10vertices/pointData/), contained files point data. 
#The seventh arguments is number of initial points in each sub partitions for domain initialization. (we choose 5)
#The eighth arguments is number of initial points in each sub partitions for subpartition initialization. (we choose 10)

if [ -z "$(ls -A $7/$9/)" ]; then
	if [ $6 -eq 0 ]; then
		echo "There is no data points, you should set the fifth argument = 1"
		exit 1
	else
		echo "./bin/nonUniformDistribute $1 $1 $2 $2 $3 $7/$9/ 10 15"
		./bin/nonUniformDistribute $1 $1 $2 $2 $3 $7/$9/ 10 15
	fi
else #exist files in $7/$9
	if [ $6 -eq 1 ]; then
		rm -rf $7/$9
		mkdir $7/$9
		echo "./bin/nonUniformDistribute $1 $1 $2 $2 $3 $7/$9/ 10 15"
		./bin/nonUniformDistribute $1 $1 $2 $2 $3 $7/$9/ 10 15
	fi
fi


#2. Run Delaunay in parallel using MPI. List of node IP address in file machinefile
#compile: make
if [ -z "$(ls -A $7/$9/)" ]; then
	echo "There is no data points, please run again !!!"
	exit 1
else
	echo "mpiexec -n $5 -f machinefile ./bin/TIPP $7/$9/ $8/$9/ `expr $1 \* $2` $4"
	mpiexec -n $5 -f machinefile ./bin/TIPP $7/$9/ $8/$9/ `expr $1 \* $2` $4
fi


#This folder $mainDir/$1/delaunayResults/ also contains the result of Delaunay (triangleIds.tri)

#3. Check Correct
#mpiexec -n $4 -f machinefile ./testDelaunay $mainDir/$1/delaunayResults/ $2 $3

#4. Draw Delaunay result (number of points should less than 20000)
#compile, g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph
#should install libgraph (see below reference)
echo "./bin/drawDomain $7/$9/ $8/$9/ `expr $1 \* $2`"
./bin/drawDomain $7/$9/ $8/$9/ `expr $1 \* $2`
