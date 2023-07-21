clang -O3 -g -mavx2 -fuse-ld=lld -Wl,--icf=none cache_conflicts.c -DLIKWID_PERFMON -o cache_conflicts -llikwid
