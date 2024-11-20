#!/bin/bash

# Helper script to collect time data

# use standard likwid library default
use_builtin_likwid=false

if [ x$1 != x ]; then
    # when built with likwid within repo
    use_builtin_likwid=true
fi

res="seq-result.txt"
sum_res="seq-result_sum.txt"
div_res="seq-result_div.txt"

if [ $use_builtin_likwid = false ]; then
    echo "running with likwid-perfctr"
    likwid-perfctr -g CLOCK -c 0 -m ./sequential_test | tee $res
    egrep 'Region (SUM|DIV)|RDTSC Runtime' $res | paste -d ' ' - - | grep SUM | awk -F '[ _]' '{print $3 "," $15}' > $sum_res
    egrep 'Region (SUM|DIV)|RDTSC Runtime' $res | paste -d ' ' - - | grep DIV | awk -F '[ _]' '{print $3 "," $15}' > $div_res
else
    echo "running without likwid-perfctr"
    ./sequential_test | tee $res
    egrep 'Region|, runtime ' $res | paste -d ' ' - - | grep SUM | awk -F '[ _]' '{print $3 "," $13}' > $sum_res
    egrep 'Region|, runtime ' $res | paste -d ' ' - - | grep DIV | awk -F '[ _]' '{print $3 "," $13}' > $div_res
fi

python plot.py $sum_res $div_res
