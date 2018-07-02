#!/bin/tcsh

source /etc/profile.d/modules.csh
module purge
module load intel/17.0.1 impi/2017.1.132

icc -o serial serial.c
mpicc -o parallel parallel.c
