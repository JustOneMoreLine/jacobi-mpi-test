#!/bin/bash
#SBATCH -o np-8-1024.out
#SBATCH -p batch
#SBATCH -N 1
#SBATCH --nodelist=node-04

mpirun --mca btl_tcp_if_exclude docker0,lo -np 8 jacobi-np8-1024 -- 1024 10000

