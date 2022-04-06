export OMP_NUM_THREADS=8

pushd LULESH/build
strace -c ./lulesh2.0
read -p "Press any key"
strace ./lulesh2.0 2> strace.out
read -p "Press any key"
strace -k ./lulesh2.0 2> strace2.out
popd
