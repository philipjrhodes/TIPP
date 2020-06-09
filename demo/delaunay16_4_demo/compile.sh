g++ -std=gnu++11 -O3 -w common.cpp point.cpp boundingBox.cpp distribute.cpp distributedMain.cpp -o distribute
cd ../convertFile
g++ -std=c++11 -O3 -w qHullConvert2DFile1.cpp -o qHullConvert2DFile1
cd ../delaunay16_1

