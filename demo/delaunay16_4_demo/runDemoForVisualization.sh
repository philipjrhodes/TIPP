#Generate Data
rm ../dataSources/10Kvertices/delaunayResults/*.*
./distribute 1 4 4 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 9 10

#Run Delaunay Triangulation
mpiexec -n 4 -f machinefile ./TIPP "../dataSources/10Kvertices/" 1

#run graphic to visulize images
./drawDomain ../dataSources/10Kvertices/ 1
