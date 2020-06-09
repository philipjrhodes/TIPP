#250M
rm ../dataSources/250Mvertices/delaunayResults/*.*
./distribute 1 8 8 4 4 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1

rm ../dataSources/250Mvertices/delaunayResults/*.*
./distribute 1 8 8 8 8 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1


rm ../dataSources/250Mvertices/delaunayResults/*.*
./distribute 1 16 16 8 8 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1

rm ../dataSources/250Mvertices/delaunayResults/*.*
./distribute 1 8 8 16 16 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1

rm ../dataSources/250Mvertices/delaunayResults/*.*
./distribute 1 16 16 16 16 "../dataSources/250Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/250Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 1



#500M
rm ../dataSources/500Mvertices/delaunayResults/*.*
./distribute 1 8 8 4 4 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1

rm ../dataSources/500Mvertices/delaunayResults/*.*
./distribute 1 8 8 8 8 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1


rm ../dataSources/500Mvertices/delaunayResults/*.*
./distribute 1 16 16 8 8 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1

rm ../dataSources/500Mvertices/delaunayResults/*.*
./distribute 1 8 8 16 16 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1

rm ../dataSources/500Mvertices/delaunayResults/*.*
./distribute 1 16 16 16 16 "../dataSources/500Mvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/500Mvertices/delaunayResults/" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 1






#sudo pkill -9 domain

