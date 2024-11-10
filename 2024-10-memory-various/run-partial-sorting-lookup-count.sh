
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 500 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 1000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 2000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 5000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 10000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 20000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 50000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 100000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 200000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 500000 -b 8"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./partial_sorting -s 10000000 -v 1000000 -b 8"

