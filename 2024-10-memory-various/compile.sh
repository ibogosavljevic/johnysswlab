g++ -DLIKWID_PERFMON -mavx2 -O3 data_lookup.cpp -o data_lookup -DLIKWID_PERDMON -llikwid
g++ -DLIKWID_PERFMON -mavx2 -O3 data_filtering.cpp -o data_filtering -DLIKWID_PERDMON -llikwid
g++ -DLIKWID_PERFMON -mavx2 -O3 matmul.cpp -o matmul -DLIKWID_PERDMON -llikwid
g++ -DLIKWID_PERFMON -mavx2 -O3 matrix_transpose.cpp -o matrix_transpose -DLIKWID_PERDMON -llikwid
g++ -DLIKWID_PERFMON -mavx2 -O3 partial_sorting.cpp -o partial_sorting -DLIKWID_PERDMON -llikwid
