#ssh r2449@hpcwoods.olemiss.edu, Gnoucnm68!
#ssh r2449@maple, Changeme12!@
#qsub -I -l select=15:ncpus=36:mem=118gb -l place=free
#qsub -I -l select=15:ncpus=36:mem=120gb -l place=free
#module load pbspro
#module load mvapich2
#qstat - f n


#g++ -std=gnu++11 -pg -O3 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp linkList.cpp -o domain

#g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp drawMesh.cpp linkList.cpp -o domain -lgraph
#mpic++ delaunayMPI.cpp linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp -o delaunayMPI




#g++ -std=gnu++11 -O3 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp linkList.cpp -o domain
#mpic++ -O3 delaunayMPI.cpp linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp -o delaunayMPI


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

#./domain "../dataSources/300vertices/" 1 4
#./domain "../dataSources/400vertices/" 1 4
#./domain "../dataSources/500vertices/" 1 4
#./domain "../dataSources/600vertices/" 1 4
#./domain "../dataSources/700vertices/" 1 4
#./domain "../dataSources/800vertices/" 1 4
#./domain "../dataSources/900vertices/" 1 4
./domain "../dataSources/1Kvertices/" 1 4
#./domain "../dataSources/1.5Kvertices/" 1 4
#./domain "../dataSources/2Kvertices/" 1 4

#./domain "../dataSources/2Kvertices/" 1 540
#./domain "../dataSources/9Kvertices/" 3 252
#./domain "../dataSources/10Kvertices/" 1 252

#./domain "../dataSources/500Kvertices/" 1 360


#./domain "../dataSources/1Mvertices/" 450

#rm -f ../dataSources/5Mvertices/delaunayResults/triangleIds.tri
#./domain "../dataSources/5Mvertices/" 360


#./domain "../dataSources/50Mvertices/" 1 252


#./domain "../dataSources/500Mvertices/" 1 216

#4 means domainSize
#if domainSize=1, then the domain is square between 0,0 - 1,1
#if domainSize=3, then the domain is square between 0,0 - 3,3
#./domain "../dataSources/4.5Bvertices/" 4 252
 

#./domain "../dataSources/500Mvertices/" 468
#gprof ./domain > gprofResult32x32.txt
#gprof ./domain > gprofResult64x64.txt
#gprof ./domain > gprofResult128x128.txt



#468 means the number of available cores in hpcwoods.olemiss.edu
#./domain "../dataSources/15Mvertices/" 468
#./domain "../dataSources/30Mvertices/" 468
#./domain "../dataSources/60Mvertices/" 360
#./domain "../dataSources/120Mvertices/" 360
#./domain "../dataSources/240Mvertices/" 360
#./domain "../dataSources/480Mvertices/" 360















#rm -f ../dataSources/100000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000000vertices/" 1

#rm -f ../dataSources/500000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/500000000vertices/" 1

#rm -f ../dataSources/1000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/1000000000vertices/" 1

#rm -f ../dataSources/2000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/2000000000vertices/" 1

#sudo pkill -9 domain

