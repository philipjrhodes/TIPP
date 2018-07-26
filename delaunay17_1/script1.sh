#1B
./distribute 2 10 8 8 8 8 "1Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10

./distribute 2 10 16 16 8 8 "1Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10


./distribute 2 10 8 8 16 16 "1Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10


./distribute 1 10 16 16 16 16 "1Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10


./distribute 1 10 32 32 16 16 "1Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10

./distribute 1 10 16 16 32 32 "1Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10
mpiexec -n 256 -f machinefile ./TIPP "1Bvertices" 2 10

rm -rf /data0/1Bvertices

#2B
./distribute 1 10 8 8 8 8 "2Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10

./distribute 1 10 16 16 8 8 "2Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10

./distribute 1 10 8 8 16 16 "2Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10

./distribute 1 10 16 16 16 16 "2Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10

./distribute 1 10 32 32 16 16 "2Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10

./distribute 1 10 16 16 32 32 "2Bvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "2Bvertices" 1 10
