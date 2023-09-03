set -x

python3 ../scripts/stat.py -n 5 -c "./main -t dfs -a builtin"
python3 ../scripts/stat.py -n 5 -c "./main -t bfs -a builtin"
python3 ../scripts/stat.py -n 5 -c "./main -t vbe -a builtin"

LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 python3 ../scripts/stat.py -n 5 -c "./main -t dfs -a builtin"
LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 python3 ../scripts/stat.py -n 5 -c "./main -t bfs -a builtin"
LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 python3 ../scripts/stat.py -n 5 -c "./main -t vbe -a builtin"

python3 ../scripts/stat.py -n 5 -c "./main -t dfs -a optimal"
python3 ../scripts/stat.py -n 5 -c "./main -t bfs -a optimal"
python3 ../scripts/stat.py -n 5 -c "./main -t vbe -a optimal"