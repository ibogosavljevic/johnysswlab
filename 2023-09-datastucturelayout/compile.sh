clang++ -Werror -DLIKWID_PERFMON -O3 -std=c++17 -Werror -g array_of_pointers.cpp -o array_of_pointers -llikwid
clang++ -Werror -DLIKWID_PERFMON -O3 -g nary_tree.cpp -o nary_tree -llikwid
clang++ -Werror -DLIKWID_PERFMON -O3 -g map_test.cpp -o map_test -llikwid -labsl_base -labsl_hash -labsl_raw_hash_set
clang++ -Werror -DLIKWID_PERFMON -O3 -g linked_list_test.cpp -o linked_list_test -llikwid
