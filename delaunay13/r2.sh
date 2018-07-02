g++ -std=gnu++11 common.cpp point.cpp edge.cpp triangle.cpp delaunay.cpp delaunayMain.cpp linkList.cpp -o delaunay -lgraph

#The last parameter is the number of partitions you want to do delaunay and print out

#rm -f ../dataSources/10vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/10vertices/" 1

#rm -f ../dataSources/100vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/100vertices/" 1

rm -f ../dataSources/1000vertices/mydatabin.tri mydatabin.tri.xfdl
./delaunay "../dataSources/1000vertices/" 9

#rm -f ../dataSources/10Kvertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/10Kvertices/" 16

#rm -f ../dataSources/100000vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/100000vertices/" 16

#rm -f ../dataSources/1Mvertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/1Mvertices/" 256

#rm -f ../dataSources/10Mvertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/10Mvertices/" 256

#rm -f ../dataSources/100000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/100000000vertices/" 1

#rm -f ../dataSources/500000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/500000000vertices/" 1

#rm -f ../dataSources/1000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/1000000000vertices/" 1

#rm -f ../dataSources/2000000000vertices/mydatabin.tri mydatabin.tri.xfdl
#./delaunay "../dataSources/2000000000vertices/" 1

#sudo pkill -9 delaunay

