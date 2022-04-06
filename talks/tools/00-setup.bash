rm -rf LULESH
git clone git@github.com:ibogosavljevic/LULESH.git
pushd LULESH
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_MPI=Off
make -j4
cd ..
mkdir build-coverage
cd build-coverage
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_MPI=Off -DWITH_OPENMP=Off -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_CXX_LINK_FLAGS="--coverage"
make -j4
popd

g++ -O3 -g -DMUL1 matmul.cpp -o matmul1
g++ -O3 -g -DMUL2 matmul.cpp -o matmul2

g++ -O3 -g flaky-bug.cpp -o flaky-bug
