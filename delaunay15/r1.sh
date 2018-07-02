#g++ -std=gnu++11 -O3 -fopenmp common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
#g++ -std=gnu++11  -fopenmp common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
g++ -std=gnu++11 common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute

#The first five arguments are domain size, and numbers of coarse-grained partitions (xCoarsePartNum, yCoarsePartNum) and fine-grained partitions (xFinePartNum, yFinePartNum) (two levels of partitioning)
#The 6th - 8th arguments are source path (../dataSources/10vertices/), point info (mydatabin.ver.xfdl), and point data (mydatabin.ver)
#The 9th argument is destination path (../dataSources/10vertices/delaunayResults/)
#The 10th is the chunkSize (read partly of dataset)
#The 11th is the number of init points of a fine-grained partition for domain,
#The 12th is the number of init points of a fine-grained partition


rm ../dataSources/10Kvertices/delaunayResults/*.*
./distribute 1 4 4 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 9 12

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



