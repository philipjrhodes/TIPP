g++ -pg -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp partition.cpp domain.cpp domainMain.cpp linkList.cpp -o domain 

g++ -pg -std=gnu++11 linkList.cpp common.cpp point.cpp edge.cpp triangle.cpp gridElement.cpp gridBound.cpp boundingBox.cpp delaunayLocal.cpp delaunayLocalMain.cpp -o delaunayLocalMain

#The last parameter is the number of partitions you want to do domain and print out

#rm -f ../dataSources/10vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/10vertices/" 1

#rm -f ../dataSources/100vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100vertices/"

#./domain "../dataSources/1Kvertices/"

#./domain "../dataSources/10Kvertices/"

#rm -f ../dataSources/10Kvertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/10Kvertices/"

#rm -f ../dataSources/100000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000vertices/" 16




./domain "../dataSources/1Mvertices/"


#./domain "../dataSources/10Mvertices/"




#rm -f ../dataSources/100000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/100000000vertices/" 1

#rm -f ../dataSources/500000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/500000000vertices/" 1

#rm -f ../dataSources/1000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/1000000000vertices/" 1

#rm -f ../dataSources/2000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./domain "../dataSources/2000000000vertices/" 1

#sudo pkill -9 domain

