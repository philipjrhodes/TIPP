g++ -std=gnu++11 common.cpp point.cpp distribute.cpp distributedMain.cpp -o distribute

#rm -f  ../dataSources/100vertices/pointPartitions/*.*
#./distribute 4 4 "../dataSources/100vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100vertices/pointPartitions/" 20 1

rm ../dataSources/1Kvertices/delaunayResults/*.*
./distribute 4 4 "../dataSources/1Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Kvertices/delaunayResults/" 1000 20


#rm ../dataSources/10Kvertices/delaunayResults/*.*
#./distribute 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 40

#rm ../dataSources/100Kvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/100Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Kvertices/delaunayResults/" 10000 100


#rm ../dataSources/500Kvertices/delaunayResults/*.*
#./distribute 4 4 "../dataSources/500Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Kvertices/delaunayResults/" 10000 25

#rm ../dataSources/500Kvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/500Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Kvertices/delaunayResults/" 10000 25

#rm ../dataSources/500Kvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/500Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Kvertices/delaunayResults/" 10000 25




#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 100

#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 50



#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 4 4 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 50


#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 10



#rm ../dataSources/10Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 100000 250

#rm ../dataSources/10Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 100000 100

#rm ../dataSources/10Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 100000 50






#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 100000 50

#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 64 64 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 100000 50



#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 300

#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 200

#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 25

#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 64 64 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 15



#rm ../dataSources/500Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 1000000 15


#rm ../dataSources/500Mvertices/delaunayResults/*.*
#./distribute 64 64 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 1000000 15


#rm ../dataSources/500Mvertices/delaunayResults/*.*
#./distribute 128 128 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 1000000 15



#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 200

#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  32 32 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 25

#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  64 64 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 20

#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  128 128 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 20




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
