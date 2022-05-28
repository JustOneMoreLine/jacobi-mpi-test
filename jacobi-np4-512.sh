#!/bin/bash
#SBATCH -o np-4-512.out
#SBATCH -p batch
#SBATCH -N 1
#SBATCH --nodelist=node-01

mpirun --mca btl_tcp_if_exclude docker0,lo -np 4 jacobi-np4-512 -- 512 10000
