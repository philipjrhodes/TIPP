#ssh r2449@hpcwoods.olemiss.edu, Gnoucnm68!
#ssh r2449@maple, Changeme12!@
#qsub -I -l select=15:ncpus=36:mem=118gb -l place=free
#qsub -I -l select=15:ncpus=36:mem=120gb -l place=free
#module load pbspro
#module load mvapich2
#qstat - f n


#g++ -std=gnu++11 -pg -O3 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp drawMesh.cpp linkList.cpp -o domain

#g++ -std=gnu++11 -O3 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp drawMesh.cpp linkList.cpp -o domain -lgraph

g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp drawMesh.cpp linkList.cpp -o domain -lgraph

mpic++ delaunayMPI.cpp linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp -o delaunayMPI


#The last parameter is the number of partitions you want to do domain and print out

#rm -f ../dataSources/10vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/10vertices/" 1

#rm -f ../dataSources/100vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100vertices/"


#./domain "../dataSources/10Kvertices/"

#rm -f ../dataSources/10Kvertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/10Kvertices/"

#rm -f ../dataSources/100000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000vertices/" 16


#rm -f ../dataSources/1Kvertices/delaunayResults/triangleIds.tri

./domain "../dataSources/1Kvertices/" 540


#./domain "../dataSources/500Kvertices/" 540


#./domain "../dataSources/1Mvertices/" 450

#./domain "../dataSources/5Mvertices/" 450


#./domain "../dataSources/50Mvertices/" 540
 

#./domain "../dataSources/500Mvertices/" 540
#gprof ./domain > gprofResult64x64.txt
#gprof ./domain > gprofResult128x128.txt


#rm -f ../dataSources/100000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000000vertices/" 1

#rm -f ../dataSources/500000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/500000000vertices/" 1

#rm -f ../dataSources/1000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/1000000000vertices/" 1

#rm -f ../dataSources/2000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/2000000000vertices/" 1

#sudo pkill -9 domain

