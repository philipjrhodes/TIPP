#250M
./distribute 1 10 8 8 4 4 "250Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10

./distribute 1 10 8 8 8 8 "250Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10

./distribute 1 10 16 16 8 8 "250Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10

./distribute 1 10 8 8 16 16 "250Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "250Mvertices" 1 10



#500M
./distribute 1 10 8 8 8 8 "500Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10

./distribute 1 10 16 16 8 8 "500Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10

./distribute 1 10 8 8 16 16 "500Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10

./distribute 1 10 16 16 16 16 "500Mvertices" "mydatabin.ver.xfdl" "mydatabin.ver" 10000000 10 15
mpiexec -n 256 -f machinefile ./TIPP "500Mvertices" 1 10


