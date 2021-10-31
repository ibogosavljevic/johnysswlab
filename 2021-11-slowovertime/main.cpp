#define LIKWID_PERFMON

#include <likwid.h>
#include <unordered_set>
#include <cstdlib>
#include <iostream>

int main(int argc, char** argv) {
    static constexpr int start_size = 64;
    static constexpr int end_size = 128*1024*1024;
    static constexpr int search_count = 8* 1024 * 1024;

    std::cout << "Performing " << search_count << " searches\n";

    LIKWID_MARKER_INIT;

    for (int size = start_size; size <= end_size; size *= 2) {
        std::unordered_set<int> s;

        std::string name = "size_" + std::to_string(size);
        std::cout << "Hash map size " << size << std::endl;

        for (int i = 0; i < size; i++) {
            s.insert(rand());
        }

        LIKWID_MARKER_START(name.c_str());

        int found = 0;

        for (int i = 0; i < search_count; i++) {
            found += s.count(i);
        }

        LIKWID_MARKER_STOP(name.c_str());
        std::cout << "Found " << found << std::endl;
    }

    LIKWID_MARKER_CLOSE;
}