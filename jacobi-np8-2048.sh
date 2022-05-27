#!/bin/bash
#SBATCH -o np-8-2048.out
#SBATCH -p batch
#SBATCH -N 8
#SBATCH --nodelist=node-01,node-02,node-03,node-04,node-05,node-06,node-07,node-08

mpirun --mca btl_tcp_if_exclude docker0,lo -np 8 jacobi-np8-2048 -- 2048 10000