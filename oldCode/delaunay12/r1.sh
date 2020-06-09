#!/bin/csh
cd /share/newDelaunay2D/dataSources/
rm -rf $1
mkdir $1
mkdir $1/rawPointData
mkdir $1/delaunayResults
echo "Generating mydata.ver from qHull ..."
rbox $2 n D2 O0.5 > $1/rawPointData/mydata.ver 
echo "Converting text data mydata.ver to mydatabin.ver ..."
../convertFile/qHullConvert2DFile1 $1/rawPointData/
echo "remove text file mydata.ver"
rm $1/rawPointData/mydata.ver


