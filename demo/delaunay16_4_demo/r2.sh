#./distribute 4 4 4 4 10000 "../dataSources/10Kvertices/delaunayResults/" 5 10
#The first four arguments are numbers of partition rows and partition columns of coarse partition and fine partitions
#the fifth argument is the number of points in doamin
#The 6th - 8th arguments are source path (../dataSources/10vertices/), init points in each fine partition for domain, and init points in each fine partition for coarse partition

#mpiexec -n 4 -f machinefile ../testDelaunay2D/testDelaunay "../dataSources/10Kvertices/delaunayResults/" 16 1000
#this command is the test (add all triangle areas)
#number 4 after -n mean 4 processes
#The last two arguments are domain size (xCoasrsePartNum* xFinePartNum), and 1000 mean segment for triangleIds.tri


#old distribution (real)
#rm -rf ../dataSources/10Kvertices/delaunayResults
#mkdir ../dataSources/10Kvertices/delaunayResults
#./distribute 1 4 4 4 4 "../dataSources/10Kvertices/rawPointData/" "mydatabin.ver.xfdl" "mydatabin.ver" "../dataSources/10Kvertices/delaunayResults/" 10000 2 20
#mpiexec -n 4 -f machinefile ./TIPP "../dataSources/10Kvertices/" 1


#new distribution (make up)
#rm -rf ../dataSources/10Kvertices/delaunayResults
#mkdir ../dataSources/10Kvertices/delaunayResults
#./distribute 4 4 4 4 10000 "../dataSources/10Kvertices/delaunayResults/" 5 10
#mpiexec -n 4 -f machinefile ./TIPP "../dataSources/10Kvertices/" 16
#mpiexec -n 4 -f machinefile ../testDelaunay2D/testDelaunay "../dataSources/10Kvertices/delaunayResults/" 16 1000
#./drawDomain ../dataSources/10Kvertices/ 16



#========================== 250M ===============================
#rm -rf ../dataSources/250Mvertices/delaunayResults
#mkdir ../dataSources/250Mvertices
#mkdir ../dataSources/250Mvertices/delaunayResults
#./distribute 8 8 8 8 250000000 "../dataSources/250Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/250Mvertices/" 64


#rm -rf ../dataSources/250Mvertices/delaunayResults
#mkdir ../dataSources/250Mvertices
#mkdir ../dataSources/250Mvertices/delaunayResults
#./distribute 8 8 16 16 250000000 "../dataSources/250Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128


#rm -rf ../dataSources/250Mvertices/delaunayResults
#mkdir ../dataSources/250Mvertices
#mkdir ../dataSources/250Mvertices/delaunayResults
#./distribute 16 16 8 8 250000000 "../dataSources/250Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128



#========================== 500M ===============================
#rm -rf ../dataSources/500Mvertices/delaunayResults
#mkdir ../dataSources/500Mvertices
#mkdir ../dataSources/500Mvertices/delaunayResults
#./distribute 8 8 8 8 500000000 "../dataSources/500Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/500Mvertices/" 64

#rm -rf ../dataSources/500Mvertices/delaunayResults
#mkdir ../dataSources/500Mvertices
#mkdir ../dataSources/500Mvertices/delaunayResults
#./distribute 8 8 16 16 500000000 "../dataSources/500Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128


#rm -rf ../dataSources/500Mvertices/delaunayResults
#mkdir ../dataSources/500Mvertices
#mkdir ../dataSources/500Mvertices/delaunayResults
#./distribute 16 16 8 8 500000000 "../dataSources/500Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128


#rm -rf ../dataSources/500Mvertices/delaunayResults
#mkdir ../dataSources/500Mvertices
#mkdir ../dataSources/500Mvertices/delaunayResults
#./distribute 16 16 16 16 500000000 "../dataSources/500Mvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/500Mvertices/" 256



#========================== 1B ===============================
#rm -rf ../dataSources/1Bvertices/delaunayResults
#mkdir ../dataSources/1Bvertices
#mkdir ../dataSources/1Bvertices/delaunayResults
#./distribute 8 8 16 16 1000000000 "../dataSources/1Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
#mpiexec -n 512 -f machinefile ../testDelaunay2D/testDelaunay "../dataSources/1Bvertices/delaunayResults/" 128 100000000

#rm -rf ../dataSources/1Bvertices/delaunayResults
#mkdir ../dataSources/1Bvertices
#mkdir ../dataSources/1Bvertices/delaunayResults
#./distribute 16 16 8 8 1000000000 "../dataSources/1Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128


#rm -rf ../dataSources/1Bvertices/delaunayResults
#mkdir ../dataSources/1Bvertices
#mkdir ../dataSources/1Bvertices/delaunayResults
#./distribute 16 16 16 16 1000000000 "../dataSources/1Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/1Bvertices/" 256


#rm -rf ../dataSources/1Bvertices/delaunayResults
#mkdir ../dataSources/1Bvertices
#mkdir ../dataSources/1Bvertices/delaunayResults
#./distribute 16 16 32 32 1000000000 "../dataSources/1Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/1Bvertices/" 512



#========================== 2B ===============================
#rm -rf ../dataSources/2Bvertices/delaunayResults
#mkdir ../dataSources/2Bvertices
#mkdir ../dataSources/2Bvertices/delaunayResults
#./distribute 16 16 8 8 2000000000 "../dataSources/2Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/2Bvertices/" 128


#rm -rf ../dataSources/2Bvertices/delaunayResults
#mkdir ../dataSources/2Bvertices
#mkdir ../dataSources/2Bvertices/delaunayResults
#./distribute 16 16 16 16 2000000000 "../dataSources/2Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/2Bvertices/" 256

#rm -rf ../dataSources/2Bvertices/delaunayResults
#mkdir ../dataSources/2Bvertices
#mkdir ../dataSources/2Bvertices/delaunayResults
#./distribute 16 16 32 32 2000000000 "../dataSources/2Bvertices/delaunayResults/" 10 15
#mpiexec -n 512 -f machinefile ./TIPP "../dataSources/2Bvertices/" 512



