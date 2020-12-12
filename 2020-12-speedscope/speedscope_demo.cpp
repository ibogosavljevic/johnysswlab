#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <parallel/algorithm>
#include <execution>

std::vector<std::string> load_file(std::string file_name) {
    std::vector<std::string> result;
    std::ifstream infile(file_name);
    std::string line;

    result.reserve(10000);
    while (std::getline(infile, line)) {
        result.push_back(line);
    }

    return result;
}

void sort_lines(std::vector<std::string>& lines) {
#if __cplusplus >= 201703L
    std::sort(std::execution::par_unseq, lines.begin(), lines.end());
#else
    __gnu_parallel::sort(lines.begin(), lines.end());
#endif
}

void remove_duplicates_and_save(std::vector<std::string>& lines,
                                std::string file_name) {
    std::ofstream myfile(file_name);

    if (lines.size() == 0) {
        return;
    }
    myfile << lines[0] << '\n';
    for (int i = 1; i < lines.size(); i++) {
        if (lines[i] != lines[i - 1]) {
            myfile << lines[i] << '\n';
        }
    }
}

void generate_files(int line_count, std::string file_name) {
    std::ofstream myfile(file_name);

    for (int i = 0; i < line_count; i++) {
        myfile << (rand() % line_count) << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string in_file(argv[1]);
    std::string out_file(argv[2]);

    // generate_files(1000000, "out.txt");
    // return 0;

    std::vector<std::string> lines = load_file(in_file);
    sort_lines(lines);
    remove_duplicates_and_save(lines, out_file);

    return 0;
}