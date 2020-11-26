make -C ../../2020-08-memoryaccess/ clean
make -C ../../2020-08-memoryaccess/

../../2020-08-memoryaccess/main -t stdset -a optimal
../../2020-08-memoryaccess/main -t dfs -a optimal
../../2020-08-memoryaccess/main -t bfs -a optimal
../../2020-08-memoryaccess/main -t bfs -a optimal -l 1

perf stat -e instructions,cycles,LLC-load-misses,LLC-loads,L1-dcache-load-misses,L1-dcache-loads,dTLB-load-misses,dTLB-loads ../../2020-08-memoryaccess/main -t stdset -a optimal
perf stat -e instructions,cycles,LLC-load-misses,LLC-loads,L1-dcache-load-misses,L1-dcache-loads,dTLB-load-misses,dTLB-loads ../../2020-08-memoryaccess/main -t bfs -a optimal
perf stat -e instructions,cycles,LLC-load-misses,LLC-loads,L1-dcache-load-misses,L1-dcache-loads,dTLB-load-misses,dTLB-loads ../../2020-08-memoryaccess/main -t dfs -a optimal
perf stat -e instructions,cycles,LLC-load-misses,LLC-loads,L1-dcache-load-misses,L1-dcache-loads,dTLB-load-misses,dTLB-loads ../../2020-08-memoryaccess/main -t dfs -a optimal -l 1
