#!/bin/bash
#SBATCH -o run-1.out
#SBATCH -p batch
#SBATCH -N 4
#SBATCH --nodelist=node-01,node-02,node-03,node-04

mpirun --mca btl_tcp_if_exclude docker0,lo -np 4 /home/user19/soal-01

