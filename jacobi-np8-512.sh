#!/bin/bash
#SBATCH -o np-8-512.out
#SBATCH -p batch
#SBATCH -N 1
#SBATCH --nodelist=node-02

mpirun --mca btl_tcp_if_exclude docker0,lo -np 8 jacobi-np8-512 -- 512 10000
