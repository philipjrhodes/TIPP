#!/bin/bash 

#The first argument is the number of partitions on both sizes (4) (two sizes of domain are equal). Domain sizes are 2^n (4, 8, 16, 32, 64, 128, ....). #If we have domain size=8, then then we have 8x8=64 partitons.

#The second argument is number of points in the domain

#The third arguments is the number of processes.

#The fourth arguments (1 or 0) meaning generate point data or not.

#The fifth, sixth, seventh arguments are source path (contain point files), destination path (contain triangulation results triangleIds.tri), and folderName. (source: ../dataSources/10Kvertices, and results: ../dataSources/10Kvertices)

#The eighth argument is the option1 --> (uniform/nonUniform)

#The nineth argument is the option2 --> (sharedFolder/nonSharedFolder)


#echo ${10}

if [ "$#" != 9 ]; then
	echo "number of arguments should be 9"
	exit
fi

if [ "$9" != "nonSharedFolder" -a  "$9" != "sharedFolder" ]; then
	echo "are you running with sharedFolder or nonSharedFolder option?"
	exit
fi

if [ "$8" != "nonUniform" -a  "$8" != "uniform" ]; then
	echo "are you running uniform or nonUniform?"
	exit
fi

if [ ! -d $5 -o ! -d $6 ]; then #not exist folder $5 or $6
	mkdir $5
	mkdir $5/$7
	if [ "$5" != "$6" ]; then
		mkdir $6
		mkdir $6/$7
	fi
else
	if [ ! -d $5/$7 ]; then #not exist folder $7 in $6
		mkdir $5/$7
	else #exist folder $7 in $5
		if [ $4 -eq 1 ]; then #need to generate the new data files
			rm -rf $5/$7
			mkdir $5/$7
		fi
	fi

	if [ ! -d $6/$7 ]; then #not exist folder $7 in $6
		mkdir $6/$7
	else #exist folder $7 in $6
		if [ "$5" != "$6" ]; then
			rm -rf $6/$7
			mkdir $6/$7
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

if [ "$8" = "nonUniform" ]; then #nonUniform
	if [ -z "$(ls -A $5/$7)" ]; then #nothing in $5/$7
		if [ $4 -eq 0 ]; then
			echo "There is no data points, you should set the fourth argument = 1"
			exit 1
		else
			echo "./bin/nonUniformDistribute $1 $1 $2 $5/$7/ 25"
			./bin/nonUniformDistribute $1 $1 $2 $5/$7/ 25
			#/home/cc/freeCluster
		fi
	else #exist files in $5/$7
		if [ $4 -eq 1 ]; then
			rm -rf $5/$7
			mkdir $5/$7
			echo "./bin/nonUniformDistribute $1 $1 $2 $5/$7/ 25"
			./bin/nonUniformDistribute $1 $1 $2 $5/$7/ 25
			#/home/cc/freeCluster
		fi
	fi
fi


if [ "$8" = "uniform" ]; then #nonUniform
	if [ -z "$(ls -A $5/$7)" ]; then #nothing in $5/$7
		if [ $4 -eq 0 ]; then
			echo "There is no data points, you should set the fourth argument = 1"
			exit 1
		else
			echo "./bin/uniformDistribute $1 $1 $2 $5/$7/ 25"
			./bin/uniformDistribute $1 $1 $2 $5/$7/ 25
			#/home/cc/freeCluster
		fi
	else #exist files in $5/$7
		if [ $4 -eq 1 ]; then
			rm -rf $5/$7
			mkdir $5/$7
			echo "./bin/uniformDistribute $1 $1 $2 $5/$7/ 25"
			./bin/uniformDistribute $1 $1 $2 $5/$7/ 25
			#/home/cc/freeCluster
		fi
	fi
fi



#2. Run Delaunay in parallel using MPI. List of node IP address in file machinefile
#compile: make
if [ -z "$(ls -A $5/$7)" ]; then
	echo "There is no data points, please run again !!!"
	exit 1
else
	rm $6/$7/triangleIds.*
	if [ "$9" = "nonSharedFolder" -o "$9" = "sharedFolder" ]; then
		if [ "$9" = "nonSharedFolder" ]; then
			echo "mpiexec -n $3 --machinefile machinefile ./bin/TIPP $5/$7/ $6/$7 $1 0"
 			#mpiexec -n $3 -f machinefile ./bin/TIPP $5/$7/ $6/$7 $1 0
 			mpiexec -n $3 --machinefile machinefile ./bin/TIPP $5/$7/ $6/$7 $1 0
		else 
			if [ "$9" = "sharedFolder" ]; then
				echo "mpiexec -n $3 --machinefile machinefile ./bin/TIPP $5/$7/ $6/$7 $1 1"				
		 		#mpiexec -n $3 -f machinefile ./bin/TIPP $5/$7/ $6/$7 $1 1
		 		mpiexec -n $3 --machinefile machinefile ./bin/TIPP $5/$7/ $6/$7 $1 1
			fi
		fi
	else
		echo "TIPP hasn't run yet!"
		exit
	fi
fi


#3. Check Correct
#mpiexec -n $4 -f machinefile ./testDelaunay $mainDir/$1/delaunayResults/ $2 $3

#4. pre process data
echo "./bin/posProcessData $5/$7/ $6/$7/ $1 $1 10000000"
#./bin/posProcessData $6/$8/ $7/$8/ $1 $1 10000000


#5. Draw Delaunay result (number of points should less than 20000)
#compile, g++ -std=gnu++11 point.cpp edge.cpp triangle.cpp boundingBox.cpp drawMesh.cpp drawDomain.cpp -o drawDomain -lgraph
#should install libgraph (see below reference)
echo "./bin/drawDomain $5/$7/ $6/$7/ $1 $1 3"
./bin/drawDomain $5/$7/ $6/$7/ $1 $1 3


#sudo apt-get install build-essential
#sudo apt-get install libsdl-image1.2 libsdl-image1.2-dev guile-1.8 guile-1.8-dev libsdl1.2debian libart-2.0-dev libaudiofile-dev libesd0-dev libdirectfb-dev libdirectfb-extra libfreetype6-dev libxext-dev x11proto-xext-dev libfreetype6 libaa1 libaa1-dev libslang2-dev libasound2 libasound2-dev

#install libgraph, download from
#download.savannah.gnu.org/releases/libgraph/libgraph-1.0.2.tar.gz
#cd libgraph-1.0.2
#./configure
#sudo make
#sudo make install
#sudo cp /usr/local/lib/libgraph.* /usr/lib



