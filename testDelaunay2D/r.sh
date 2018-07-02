#three parameters are: path to the triangulation data, domain size (size of one side), and the segmentSize
#segmentSize must be less than the number of triangles in the domain (size of trianglesIds.tri in bytes /8/3 --> divided by 24)
#The segment size must be divisible to number of processes.

#mpic++ -std=gnu++11 testDelaunay.cpp -o testDelaunay
#The second argument id domainSize, and the third argument is segmentSize
#mpiexec -n 20 -f machinefile ./testDelaunay "../dataSources/100vertices/delaunayResults/" 1.0 100
mpiexec -n 4 -f machinefile ./testDelaunay "../dataSources/1Kvertices/delaunayResults/" 1.0 1000
#mpiexec -n 4 -f machinefile ./testDelaunay "../dataSources/4Kvertices/delaunayResults/" 1.0 1000
#mpiexec -n 5 -f machinefile ./testDelaunay "../dataSources/5Mvertices/delaunayResults/" 1.0 100000

