#!/bin/bash 
#small sample
#./runUniform.sh 4 4 10000 4 1 ../dataSources ../delaunayResults 10Kvertices nonSharedFolder

#The first two arguments are the domain sizes of major (coarse) and minor (fine) partitions
#4x4x4x4
#(two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=4, then then we have 4x4=16 partitons

#The third argument is numper of points (10000)
#The fourth arguments is the number of processes.
#The fifth argument is the generated option (1), it has obnly 2 values 0 or 1. 1 means to regenerate the point dataset, 0 means reuse the dataset available.
#The sixth and seventh arguments are the data source folder (../dataSources) and triangulation results folder (../triangulationResults).
#The last argument is the folder name (source:  ../dataSources/10Kvertices, results:  ../dataSources/10Kvertices)
#The last argument is folder name (10Kvertices). The point data located in the ../dataSources/10Kvertices/, and the delaunay results (*.tri) will be located in ../delaunayResults/10Kvertices/

if [ "$#" != 9 ]; then
	echo "number of arguments should be 9"
	exit
fi

if [ "$9" != "nonSharedFolder" -a  "$9" != "sharedFolder" ]; then
	echo "are you running with sharedFolder or nonSharedFolder option?"
	exit
fi

#not exist folder $6 or $7
if [ ! -d $6 -o  ! -d $7 ]; then
	mkdir $6
	mkdir $6/$8
	if [ "$6" != "$7" ]; then
		mkdir $7
	mkdir $7/$8
	fi
else
	if [ ! -d $6/$8 ]; then #not exist $8 in $6
		mkdir $6/$8/
	else
		if [ $5 -eq 1 ]; then
			rm -rf $6/$8
			mkdir $6/$8
		fi
	fi

	if [ ! -d $7/$8 ]; then #not exist $8 in $7
		mkdir $7/$8
	else
		if [ "$6" != "$7" ]; then
			rm -rf $7/$8
			mkdir $7/$8
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
#The fifth argument is numper of points ($3) in the domain (ex: 10000),
#The sixth arguments is source path (ex:  ../dataSources/10vertices), contained point data files. 
#The seventh arguments is number of initial points in each sub partitions for domain initialization. (we choose 5)
#The eighth arguments is number of initial points in each sub partitions for subpartition initialization. (we choose 10)
if [ -z "$(ls -A $6/$8/)" ]; then
	if [ $5 -eq 0 ]; then
		echo "There is no data points, you should set the fifth argument = 1"
		exit 1
	else
		echo "./bin/uniformDistribute $1 $1 $2 $2 $3 $6/$8/ 5 10"
		./bin/uniformDistribute $1 $1 $2 $2 $3 $6/$8/ 5 10
		#/home/cc/freeCluster
	fi
else #exist files in $6/$8
	if [ $5 -eq 1 ]; then
		rm -rf $6/$8
		mkdir $6/$8
		echo "./bin/uniformDistribute $1 $1 $2 $2 $3 $6/$8/ 5 10"
		./bin/uniformDistribute $1 $1 $2 $2 $3 $6/$8/ 5 10
		#/home/cc/freeCluster
	fi
fi


#2. Run Delaunay in parallel using MPI. List of node IP address in file machinefile
#compile: make
if [ -z "$(ls -A $6/$8/)" ]; then
	echo "There is no data points, please run again !!!"
	exit 1
else
	if [ "$9" = "nonSharedFolder" -o "$9" = "sharedFolder" ]; then
		if [ "$9" = "nonSharedFolder" ]; then
			echo "mpiexec -n $4 -f machinefile ./bin/TIPP_NON_SHARED_FOLDER $6/$8/ $7/$8/ `expr $1 \* $2` 0"
 			mpiexec -n $4 -f machinefile ./bin/TIPP_NON_SHARED_FOLDER $6/$8/ $7/$8/ `expr $1 \* $2` 0
		else 
			if [ "$9" = "sharedFolder" ]; then
				echo "mpiexec -n $4 -f machinefile ./bin/TIPP $6/$8/ $7/$8/ `expr $1 \* $2` 0"
		 		mpiexec -n $4 -f machinefile ./bin/TIPP $6/$8/ $7/$8/ `expr $1 \* $2` 0
			fi
		fi
	else
		echo "TIPP hasn't run yet!"
		exit
	fi
fi

#This folder $mainDir/$1/delaunayResults/ also contains the result of Delaunay (triangleIds.tri)

#3. Check Correct
#mpiexec -n $4 -f machinefile ./testDelaunay $mainDir/$1/delaunayResults/ $2 $3

#4. Draw Delaunay result (number of points should less than 20000)
#compile, g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph
#should install libgraph (see below reference)
echo "./bin/drawDomain $6/$8/ $7/$8/ `expr $1 \* $2`"
./bin/drawDomain $6/$8/ $7/$8/ `expr $1 \* $2`




