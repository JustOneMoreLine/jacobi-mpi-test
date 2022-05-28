#!/bin/bash
#SBATCH -o jacobi-np4-2048.out
#SBATCH -p batch
#SBATCH -N 1
#SBATCH --nodelist=node-03

mpirun --mca btl_tcp_if_exclude docker0,lo -np 5 jacobi-mpi 2048 100

