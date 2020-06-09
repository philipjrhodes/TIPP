#250M
rm -rf ../dataSources/250Mvertices/delaunayResults
mkdir ../dataSources/250Mvertices/delaunayResults
./distribute 1 8 8 8 8 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1


rm -rf ../dataSources/250Mvertices/delaunayResults
mkdir ../dataSources/250Mvertices/delaunayResults
./distribute 1 8 8 16 16 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1

rm -rf ../dataSources/250Mvertices/delaunayResults
mkdir ../dataSources/250Mvertices/delaunayResults
./distribute 1 16 16 8 8 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1


#500M
rm -rf ../dataSources/500Mvertices/delaunayResults
mkdir ../dataSources/500Mvertices/delaunayResults
./distribute 1 8 8 8 8 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1


rm -rf ../dataSources/500Mvertices/delaunayResults
mkdir ../dataSources/500Mvertices/delaunayResults
./distribute 1 8 8 16 16 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1

rm -rf ../dataSources/500Mvertices/delaunayResults
mkdir ../dataSources/500Mvertices/delaunayResults
./distribute 1 16 16 8 8 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1

rm -rf ../dataSources/500Mvertices/delaunayResults
mkdir ../dataSources/500Mvertices/delaunayResults
./distribute 1 16 16 16 16 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1


#1B
rm -rf ../dataSources/1Bvertices/delaunayResults
mkdir ../dataSources/1Bvertices/delaunayResults
./distribute 2 8 8 16 16 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2


rm -rf ../dataSources/1Bvertices/delaunayResults
mkdir ../dataSources/1Bvertices/delaunayResults
./distribute 2 16 16 8 8 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2


rm -rf ../dataSources/1Bvertices/delaunayResults
mkdir ../dataSources/1Bvertices/delaunayResults
./distribute 2 16 16 16 16 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2




#2B
rm -rf ../dataSources/2Bvertices/delaunayResults
mkdir ../dataSources/2Bvertices/delaunayResults
./distribute 2 8 8 16 16 "../dataSources/2Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2Bvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2


rm -rf ../dataSources/2Bvertices/delaunayResults
mkdir ../dataSources/2Bvertices/delaunayResults
./distribute 2 16 16 8 8 "../dataSources/2Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2Bvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2


rm -rf ../dataSources/2Bvertices/delaunayResults
mkdir ../dataSources/2Bvertices/delaunayResults
./distribute 2 16 16 16 16 "../dataSources/2Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2Bvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2

