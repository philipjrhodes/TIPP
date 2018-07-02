rm ../dataSources/1Mvertices/delaunayResults/*.*
./distribute 4 4 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 50
./domain "../dataSources/1Mvertices/"

rm ../dataSources/1Mvertices/delaunayResults/*.*
./distribute 8 8 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 50
./domain "../dataSources/1Mvertices/"

rm ../dataSources/1Mvertices/delaunayResults/*.*
./distribute 16 16 "../dataSources/1Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Mvertices/delaunayResults/" 100000 50
./domain "../dataSources/1Mvertices/"



#############################################################
rm ../dataSources/10Mvertices/delaunayResults/*.*
./distribute 8 8 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 1000000 50
./domain "../dataSources/10Mvertices/"

rm ../dataSources/10Mvertices/delaunayResults/*.*
./distribute 16 16 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 1000000 50
./domain "../dataSources/10Mvertices/"

rm ../dataSources/10Mvertices/delaunayResults/*.*
./distribute 32 32 "../dataSources/10Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Mvertices/delaunayResults/" 1000000 50
./domain "../dataSources/10Mvertices/"


#############################################################
rm ../dataSources/100Mvertices/delaunayResults/*.*
./distribute 16 16 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 50
./domain "../dataSources/100Mvertices/"

rm ../dataSources/100Mvertices/delaunayResults/*.*
./distribute 32 32 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 50
./domain "../dataSources/100Mvertices/"

rm ../dataSources/100Mvertices/delaunayResults/*.*
./distribute 64 64 "../dataSources/100Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/100Mvertices/delaunayResults/" 1000000 50
./domain "../dataSources/100Mvertices/"


#############################################################
rm ../dataSources/1Bvertices/delaunayResults/*.*
./distribute  32 32 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 50
./domain "../dataSources/1BMvertices/"

rm ../dataSources/1Bvertices/delaunayResults/*.*
./distribute  64 64 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 50
./domain "../dataSources/1BMvertices/"


#rm ../dataSources/1Bvertices/delaunayResults/*.*
#./distribute  128 128 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 50
#./domain "../dataSources/1BMvertices/"





#sudo pkill -9 domain

