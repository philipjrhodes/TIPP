g++ -fopenmp -std=gnu++11 common.cpp point.cpp distribute.cpp distributedMain.cpp -o distribute

#rm -f  ../dataSources/100vertices/pointPartitions/*.*
#./distribute 4 4 "../dataSources/100vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100vertices/pointPartitions/" 20 1

rm ../dataSources/1Kvertices/delaunayResults/*.*
./distribute 4 4 "../dataSources/1Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Kvertices/delaunayResults/" 1000 20
#rm ../dataSources/1Kvertices/delaunayResults/pointPart.ver


#rm -f  ../dataSources/10Kvertices/pointPartitions/*.*
#./distribute 8 8 "../dataSources/10Kvertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/pointPartitions/" 10000 40

#rm -f  ../dataSources/100000vertices/pointPartitions/*.*
#./distribute 1 1 "../dataSources/100000vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100000vertices/pointPartitions/" 10000

#rm -f  ../dataSources/1000000vertices/pointPartitions/*.*
#./distribute 1 1 "../dataSources/1000000vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1000000vertices/pointPartitions/" 10000

#rm -f  ../dataSources/1Mvertices/pointPartitions/*.*
#./distribute 16 16 "../dataSources/1Mvertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/pointPartitions/" 100000

#rm -f  ../dataSources/10Mvertices/pointPartitions/*.*
#./distribute 16 16 "../dataSources/10Mvertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/pointPartitions/" 1000000

#rm -f  ../dataSources/100000000vertices/pointPartitions/*.*
#./distribute 32 32 "../dataSources/100000000vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100000000vertices/pointPartitions/" 10000000

#rm -f  ../dataSources/500000000vertices/pointPartitions/*.*
#./distribute 16 16 "../dataSources/500000000vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500000000vertices/pointPartitions/" 10000000

#rm -f  ../dataSources/1Bvertices/pointPartitions/*.*
#./distribute 256 256 "../dataSources/1Bvertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/pointPartitions/" 100000000 100


#rm -f  ../dataSources/2000000000vertices/pointPartitions/*.*
#./distribute 256 256 "../dataSources/2000000000vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2000000000vertices/pointPartitions/" 100000000


#note: first two arguments (for ex: 4 4) is total number of partitions (4x4)
#the third, fourth, fifth, and sixth are source folder, names files, and destination folder
#the two last argument are chunkSize and initial points
