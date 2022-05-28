#!/bin/bash
#SBATCH -o np-4-1024.out
#SBATCH -p batch
#SBATCH -N 1
#SBATCH --nodelist=node-03

mpirun --mca btl_tcp_if_exclude docker0,lo -np 4 jacobi-np4-1024 -- 1024 10000
