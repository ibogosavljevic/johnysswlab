export OMP_NUM_THREADS=1

pushd LULESH/build-coverage
./lulesh2.0
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory out
google-chrome out/index.html
popd
