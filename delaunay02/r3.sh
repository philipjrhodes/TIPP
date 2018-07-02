g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp linkList.cpp -o domain 

#g++ -std=gnu++11 -fopenmp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp linkList.cpp drawMesh.cpp -o domain -lgraph

mpic++ delaunayMPI.cpp linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp -o delaunayMPI


#The last parameter is the number of partitions you want to do domain and print out

#rm -f ../dataSources/10vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/10vertices/" 1

#rm -f ../dataSources/100vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100vertices/"

rm -f ../dataSources/1Kvertices/delaunayResults/triangleIds.tri
./domain "../dataSources/1Kvertices/"

#./domain "../dataSources/10Kvertices/"

#rm -f ../dataSources/10Kvertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/10Kvertices/"

#rm -f ../dataSources/100000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000vertices/" 16

#rm -f ../dataSources/1Mvertices/delaunayResults/triangleIds.tri

#./domain "../dataSources/500Kvertices/"

#./domain "../dataSources/5Mvertices/"


#./domain "../dataSources/50Mvertices/"


#./domain "../dataSources/100Mvertices/"


#./domain "../dataSources/500Mvertices/"


#./domain "../dataSources/1Bvertices/"


#rm -f ../dataSources/100000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000000vertices/" 1

#rm -f ../dataSources/500000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/500000000vertices/" 1

#rm -f ../dataSources/1000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/1000000000vertices/" 1

#rm -f ../dataSources/2000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/2000000000vertices/" 1

#sudo pkill -9 domain

