#!/bin/bash
#SBATCH -o np-4-512.out
#SBATCH -p batch
#SBATCH -N 4
#SBATCH --nodelist=node-01,node-02,node-03,node-04,node-05

mpirun --mca btl_tcp_if_exclude docker0,lo -np 5 jacobi-np4-512 -- 512 10000