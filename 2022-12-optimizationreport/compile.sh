clang++ -O3 -fsave-optimization-record compiler-optimizations.cpp -o compiler-optimizations -c -Rpass-missed=.* -Rpass=.* -Rpass-analysis=.* 2>optimizations-clang.txt
python3 /usr/lib/llvm-12/share/opt-viewer/opt-viewer.py compiler-optimizations.opt.yaml
open html/index.html
