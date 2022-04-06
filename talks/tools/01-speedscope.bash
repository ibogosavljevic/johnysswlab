# Running one thread only
export OMP_NUM_THREADS=1

pushd LULESH/build
perf record --call-graph dwarf -F 99 ./lulesh2.0
perf script | speedscope -
popd
