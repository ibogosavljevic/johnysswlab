make -C ../../2020-08-memoryaccess/ clean
make -C ../../2020-08-memoryaccess/

../../2020-08-memoryaccess/main -t stdset -a optimal
../../2020-08-memoryaccess/main -t dfs -a optimal
../../2020-08-memoryaccess/main -t bfs -a optimal
../../2020-08-memoryaccess/main -t bfs -a optimal -l 1
