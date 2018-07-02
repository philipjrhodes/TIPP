g++ -std=gnu++11 common.cpp point.cpp distribute.cpp distributedMain.cpp -o distribute

#The first four arguments are domain size, interval and numbers of partition rows and partition columns
#interval is the distance between additional points on 4 edges AB, BC, CD, DA of domain ABCD.
#The 5th - 7th arguments are source path (../dataSources/10vertices/), point info (mydatabin.ver.xfdl), and point data (mydatabin.ver)
#The 8th argument is destination path (../dataSources/10vertices/delaunayResults/)
#The 9th is the chunkSize
#The 10th is the number of init points in a patition


#rm -f  ../dataSources/100vertices/pointPartitions/*.*
#./distribute 4 4 "../dataSources/100vertices/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100vertices/pointPartitions/" 20 1

rm ../dataSources/1Kvertices/delaunayResults/*.*
./distribute 1 0.25 4 4 "../dataSources/1Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Kvertices/delaunayResults/" 1000 20

#rm ../dataSources/9Kvertices/delaunayResults/*.*
#./distribute 3 3 3 "../dataSources/9Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/9Kvertices/delaunayResults/" 9000 20


#rm ../dataSources/10Kvertices/delaunayResults/*.*
#./distribute 1 0.125 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 40

#rm ../dataSources/100Kvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/100Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Kvertices/delaunayResults/" 10000 100


#rm ../dataSources/500Kvertices/delaunayResults/*.*
#./distribute 4 4 "../dataSources/500Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Kvertices/delaunayResults/" 10000 15

#rm ../dataSources/500Kvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/500Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Kvertices/delaunayResults/" 10000 15

#rm ../dataSources/500Kvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/500Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Kvertices/delaunayResults/" 10000 15




#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 25

#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 25

#rm ../dataSources/1Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 25




#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 25

#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 25

#rm ../dataSources/5Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/5Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/5Mvertices/delaunayResults/" 100000 25


#rm ../dataSources/10Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 100000 250

#rm ../dataSources/10Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 100000 100

#rm ../dataSources/10Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 100000 50




#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 1 16 16 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 100000 25

#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 1000000 25

#rm ../dataSources/50Mvertices/delaunayResults/*.*
#./distribute 64 64 "../dataSources/50Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/50Mvertices/delaunayResults/" 1000000 25



#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 8 8 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 300

#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 200

#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 25

#rm ../dataSources/100Mvertices/delaunayResults/*.*
#./distribute 64 64 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 15



#rm ../dataSources/500Mvertices/delaunayResults/*.*
#./distribute 32 32 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 1000000 25

#rm ../dataSources/500Mvertices/delaunayResults/*.*
#./distribute 64 64 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 25

#rm ../dataSources/500Mvertices/delaunayResults/*.*
#./distribute 128 128 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 25

#3 is the domainSize
#rm ../dataSources/4.5Bvertices/delaunayResults/*.*
#./distribute 3 128 128 "../dataSources/4.5Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/4.5Bvertices/delaunayResults/" 10000000 25



#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute 16 16 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 200

#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  32 32 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 25

#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  64 64 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 20

#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  128 128 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 20




########################## Extra #################################

#rm ../dataSources/15Mvertices/delaunayResults/*.*
#./distribute  16 16 "../dataSources/15Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/15Mvertices/delaunayResults/" 10000000 25

#rm ../dataSources/30Mvertices/delaunayResults/*.*
#./distribute  16 16 "../dataSources/30Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/30Mvertices/delaunayResults/" 10000000 25

#rm ../dataSources/60Mvertices/delaunayResults/*.*
#./distribute  32 32 "../dataSources/60Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/60Mvertices/delaunayResults/" 10000000 25

#rm ../dataSources/120Mvertices/delaunayResults/*.*
#./distribute  32 32 "../dataSources/120Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/120Mvertices/delaunayResults/" 10000000 25

#rm ../dataSources/240Mvertices/delaunayResults/*.*
#./distribute  32 32 "../dataSources/240Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/240Mvertices/delaunayResults/" 10000000 25

#rm ../dataSources/480Mvertices/delaunayResults/*.*
#./distribute  64 64 "../dataSources/480Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/480Mvertices/delaunayResults/" 10000000 25





#####################################################################################
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
