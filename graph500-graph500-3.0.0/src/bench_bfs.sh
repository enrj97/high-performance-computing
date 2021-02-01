#!/bin/bash

srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_32x1_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_32x1_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_32x1_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=1 -o results/graph500_BFS_32x1_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_1x32_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_1x32_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_1x32_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 1 --ntasks-per-node=32 -o results/graph500_BFS_1x32_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_32x32_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_32x32_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_32x32_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=32 -o results/graph500_BFS_32x32_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=24 -o results/graph500_BFS_32x24_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=24 -o results/graph500_BFS_32x24_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=24 -o results/graph500_BFS_32x24_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=24 -o results/graph500_BFS_32x24_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=16 -o results/graph500_BFS_32x16_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=16 -o results/graph500_BFS_32x16_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=16 -o results/graph500_BFS_32x16_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=16 -o results/graph500_BFS_32x16_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=12 -o results/graph500_BFS_32x12_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=12 -o results/graph500_BFS_32x12_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=12 -o results/graph500_BFS_32x12_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=12 -o results/graph500_BFS_32x12_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=8 -o results/graph500_BFS_32x8_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=8 -o results/graph500_BFS_32x8_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=8 -o results/graph500_BFS_32x8_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=8 -o results/graph500_BFS_32x8_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=6 -o results/graph500_BFS_32x6_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=6 -o results/graph500_BFS_32x6_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=6 -o results/graph500_BFS_32x6_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=6 -o results/graph500_BFS_32x6_sf26.txt --time=05:00 ./graph500_reference_bfs 26

srun -p q_student -N 32 --ntasks-per-node=4 -o results/graph500_BFS_32x4_sf17.txt --time=05:00 ./graph500_reference_bfs 17
srun -p q_student -N 32 --ntasks-per-node=4 -o results/graph500_BFS_32x4_sf20.txt --time=05:00 ./graph500_reference_bfs 20
srun -p q_student -N 32 --ntasks-per-node=4 -o results/graph500_BFS_32x4_sf23.txt --time=05:00 ./graph500_reference_bfs 23
srun -p q_student -N 32 --ntasks-per-node=4 -o results/graph500_BFS_32x4_sf26.txt --time=05:00 ./graph500_reference_bfs 26