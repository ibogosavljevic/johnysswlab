
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 20"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 50"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 100"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 200"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 500"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 1000"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 2000"
python3 ../scripts/stat.py -n 10 -c "likwid-perfctr -g MEM -C 0 -m ./matrix_transpose -d 5000"