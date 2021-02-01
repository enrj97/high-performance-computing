#!/bin/bash

srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_custom32x1_sf17.txt --time=05:00 ./graph500_collective_custom_bfs 17
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_custom32x1_sf20.txt --time=05:00 ./graph500_collective_custom_bfs 20
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_custom32x1_sf23.txt --time=05:00 ./graph500_collective_custom_bfs 23
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_custom32x1_sf26.txt --time=05:00 ./graph500_collective_custom_bfs 26
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_custom32x1_sf29.txt --time=05:00 ./graph500_collective_custom_bfs 29

srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_custom1x32_sf17.txt --time=05:00 ./graph500_collective_custom_bfs 17
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_custom1x32_sf20.txt --time=05:00 ./graph500_collective_custom_bfs 20
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_custom1x32_sf23.txt --time=05:00 ./graph500_collective_custom_bfs 23
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_custom1x32_sf26.txt --time=05:00 ./graph500_collective_custom_bfs 26
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_custom1x32_sf29.txt --time=05:00 ./graph500_collective_custom_bfs 29

srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_custom32x32_sf17.txt --time=05:00 ./graph500_collective_custom_bfs 17
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_custom32x32_sf20.txt --time=05:00 ./graph500_collective_custom_bfs 20
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_custom32x32_sf23.txt --time=05:00 ./graph500_collective_custom_bfs 23
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_custom32x32_sf26.txt --time=05:00 ./graph500_collective_custom_bfs 26
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_custom32x32_sf29.txt --time=05:00 ./graph500_collective_custom_bfs 29