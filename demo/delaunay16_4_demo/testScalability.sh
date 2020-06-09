#============================ 250M ======================================
rm -rf ../dataSources/250Mvertices/delaunayResults
mkdir ../dataSources/250Mvertices
mkdir ../dataSources/250Mvertices/delaunayResults
./distribute 8 8 16 16 250000000 "../dataSources/250Mvertices/delaunayResults/" 10 15


#16 nodes
rm machinefile
cp machinefile16 machinefile
cat machinefile
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 512 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 1024 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh

#8 nodes
rm machinefile
cp machinefile8 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 512 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128

#4 nodes
rm machinefile
cp machinefile4 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 64 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128

#2 nodes
rm machinefile
cp machinefile2 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 32 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 64 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/250Mvertices/" 128

rm -rf ../dataSources/250Mvertices/delaunayResults
mkdir ../dataSources/250Mvertices/delaunayResults


#============================ 500M ===================================
rm -rf ../dataSources/500Mvertices/delaunayResults
mkdir ../dataSources/500Mvertices
mkdir ../dataSources/500Mvertices/delaunayResults
./distribute 16 16 8 8 500000000 "../dataSources/500Mvertices/delaunayResults/" 10 15

#16 nodes
rm machinefile
cp machinefile16 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 512 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 1024 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128

#8 nodes
rm machinefile
cp machinefile8 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 512 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128

#4 nodes
rm machinefile
cp machinefile4 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 64 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128

#2 nodes
rm machinefile
cp machinefile2 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 32 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 64 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/500Mvertices/" 128

rm -rf ../dataSources/500Mvertices/delaunayResults
mkdir ../dataSources/500Mvertices/delaunayResults


#============================ 1B ===================================
rm -rf ../dataSources/1Bvertices/delaunayResults
mkdir ../dataSources/1Bvertices
mkdir ../dataSources/1Bvertices/delaunayResults
./distribute 16 16 8 8 1000000000 "../dataSources/1Bvertices/delaunayResults/" 10 15

#16 nodes
rm machinefile
cp machinefile16 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 512 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 1024 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128

#8 nodes
rm machinefile
cp machinefile8 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 512 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128

#4 nodes
rm machinefile
cp machinefile4 machinefile
cat machinefile
/home/cc/freeCluster.sh
mpiexec -n 64 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 256 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128

#2 nodes
rm machinefile
cp machinefile2 machinefile
/home/cc/freeCluster.sh
cat machinefile
mpiexec -n 32 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 64 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128
/home/cc/freeCluster.sh
mpiexec -n 128 -f machinefile ./TIPP "../dataSources/1Bvertices/" 128

rm -rf ../dataSources/1Bvertices/delaunayResults
mkdir ../dataSources/1Bvertices/delaunayResults



