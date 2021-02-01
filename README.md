# HPC_TUWIEN
High Performance Computing Project

by

Enrico Coluccia, 12005483
Dominik Schreiber, 01326110

Executing a make command should generate 5 binaries (including the 4 binaries that were given by the reference implementation).
Our best performing algorithm is implemented in bfs_collective_custom.c, which is translated to the binary "graph500_collective_custom_bfs".
There is also a non collective implementation in bfs_custom.c, but this version performs very poorly.

Additionally there are three shell scripts (bench_bfs.sh, bench_sssp.sh and bench_bfs_custom.sh), which contain commands to start test runs with various configurations.
