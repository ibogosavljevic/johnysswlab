export OMP_NUM_THREADS=1

pushd LULESH/build
rm heaptrack*.zst
heaptrack ./lulesh2.0
heaptrack --analyze heaptrack*.zst
popd
