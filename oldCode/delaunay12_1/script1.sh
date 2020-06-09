#1B
rm ../dataSources/1Bvertices/delaunayResults/*.*
./distribute 2 0.03125 64 64 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 25
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2

rm ../dataSources/1Bvertices/delaunayResults/*.*
./distribute 2 0.015625 128 128 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 25
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2

rm ../dataSources/1Bvertices/delaunayResults/*.*
./distribute 2 0.0078125 256 256 "../dataSources/1Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/1Bvertices/delaunayResults/" 10000000 25
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 2

rm -rf ../dataSources/1Bvertices

#2B
rm ../dataSources/2Bvertices/delaunayResults/*.*
./distribute 2 0.03125 64 64 "../dataSources/2Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2Bvertices/delaunayResults/" 10000000 25
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2

rm ../dataSources/2Bvertices/delaunayResults/*.*
./distribute 2 0.015625 128 128 "../dataSources/2Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2Bvertices/delaunayResults/" 10000000 25
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2

rm ../dataSources/2Bvertices/delaunayResults/*.*
./distribute 2 0.0078125 256 256 "../dataSources/2Bvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/2Bvertices/delaunayResults/" 10000000 25
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/2Bvertices/" 2




