#!/bin/tcsh
#BSUB -J example_job
#BSUB -n 32
#BSUB -R "span[ptile=16]"
#BSUB -P SCSG0001
#BSUB -W 0:10
#BSUB -o out.%J
#BSUB -e err.%J
#BSUB -q small

source /etc/profile.d/modules.csh
module purge
module load intel impi

setenv OMP_NUM_THREADS 16

./serial
