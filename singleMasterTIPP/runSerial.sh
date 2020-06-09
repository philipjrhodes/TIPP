#!/bin/bash 

#The first argument is the number of partitions on both sizes (4) (two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....). #If we have domain size=8, then then we have 8x8=64 partitons.

#The second argument is number of points in the domain

#The third arguments is the number of processes.

#The fourth arguments (1 or 0) meaning generate point data or not.

#The fifth, sixth, seventh arguments are source path (contain point files), destination path (contain triangulation results triangleIds.tri), and folderName. (source: ../dataSources/10Kvertices, and results: ../dataSources/10Kvertices)

#The eighth argument is the option1 --> (uniform/nonUniform)

#The nineth argument is the option2 --> (producerConsumer/nonProducerConsumer)


if [ "$#" != 7 ]; then
	echo "number of arguments should be 8"
	exit
fi

if [ "$7" != "nonUniform" -a  "$7" != "uniform" ]; then
	echo "are you running uniform or nonUniform?"
	exit
fi

if [ ! -d $4 -o ! -d $5 ]; then #not exist folder $5 or $6
	mkdir $4
	mkdir $4/$5
	if [ "$4" != "$5" ]; then
		mkdir $5
		mkdir $5/$6
	fi
else
	if [ ! -d $4/$6 ]; then #not exist folder $7 in $6
		mkdir $4/$6
	else #exist folder $6 in $4
		if [ $3 -eq 1 ]; then #need to generate the new data files
			rm -rf $4/$6
			mkdir $4/$6
		fi
	fi

	if [ ! -d $5/$6 ]; then #not exist folder $7 in $6
		mkdir $5/$6
	else #exist folder $6 in $5
		if [ "$4" != "$5" ]; then
			rm -rf $5/$6
			mkdir $5/$6
		fi
	fi
fi




#1. Generate data points in each partitions, this need to use rbox from QHULL.
#Important: QHULL SHOULD BE INSTALLED BEFORE RUNNING THIS GENERATING AND DELAUNAY
#We should generate points in each partition before running the Delaunay (TIPP)

#The first two arguments are domain sizes $1 $1 (ex: 4 4). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....)
#If we have domain size=4, then then we have 4x4=16 partitons
#The third argument ($2) is number of point in the domain
#The fourth arguments is source path ($5/$7, ex:  ../dataSources/10Kvertices)
#The fifth arguments is number of initial points in each partitions. (we choose 25)

if [ "$7" = "nonUniform" ]; then #nonUniform
	if [ -z "$(ls -A $4/$6)" ]; then #nothing in $4/$6
		if [ $3 -eq 0 ]; then
			echo "There is no data points, you should set the fourth argument = 1"
			exit 1
		else
			echo "./bin/nonUniformDistribute $1 $1 $2 $4/$6/ 25"
			./bin/nonUniformDistribute $1 $1 $2 $4/$6/ 25
			#/home/cc/freeCluster
		fi
	else #exist files in $4/$6
		if [ $3 -eq 1 ]; then
			rm -rf $4/$6
			mkdir $4/$6
			echo "./bin/nonUniformDistribute $1 $1 $2 $4/$6/ 25"
			./bin/nonUniformDistribute $1 $1 $2 $4/$6/ 25
			#/home/cc/freeCluster
		fi
	fi
fi


if [ "$7" = "uniform" ]; then #nonUniform
	if [ -z "$(ls -A $4/$6)" ]; then #nothing in $4/$6
		if [ $3 -eq 0 ]; then
			echo "There is no data points, you should set the fourth argument = 1"
			exit 1
		else #allow to generate new point data 
			echo "./bin/uniformDistribute $1 $1 $2 $4/$6/ 25"
			./bin/uniformDistribute $1 $1 $2 $4/$6/ 25
			#/home/cc/freeCluster
		fi
	else #exist files in $4/$6
		if [ $3 -eq 1 ]; then
			rm -rf $4/$6
			mkdir $4/$6
			echo "./bin/uniformDistribute $1 $1 $2 $4/$6/ 25"
			./bin/uniformDistribute $1 $1 $2 $4/$6/ 25
			#/home/cc/freeCluster
		fi
	fi
fi



#2. Run serial Delaunay.
if [ -z "$(ls -A $4/$6)" ]; then
	echo "There is no data points, please run again !!!"
	exit 1
else
	rm $5/$6/*.tri
 	./bin/SERIAL_TIPP $4/$6/ $5/$6 $1
fi


#3. Check Correct
#mpiexec -n $4 -f machinefile ./testDelaunay $mainDir/$1/delaunayResults/ $2 $3

#4. pre process data
#echo "./bin/posProcessData $5/$7/ $6/$7/ $1 $1 10000000"
#./bin/posProcessData $6/$8/ $7/$8/ $1 $1 10000000


#5. Draw Delaunay result (number of points should less than 20000)
#compile, g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph
#should install libgraph (see below reference)
echo "./bin/drawDomain $4/$6/ $5/$6/ $1 $1 1"
./bin/drawDomain $4/$6/ $5/$6/ $1 $1 1


