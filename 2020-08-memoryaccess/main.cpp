#include <iostream>
#include <set>
#include "custom_allocator.h"
#include "utils.h"
#include "measure_time.h"
#include "binary_search_tree.h"
#include "argparse.h"

enum tree_type_e {
    BFS_TREE,
    DFS_TREE,
    STD_SET
};

enum allocator_type_e {
    BUILTIN,
    CUSTOM_OPTIMAL,
    CUSTOM_NONOPTIMAL
};

using namespace argparse;


bool parse_args(int argc, const char* argv[], tree_type_e& out_tree_type, allocator_type_e& out_allocator_type, bool& out_large_pages) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-t", "--tree", "Type of the tree (bfs, dfs, stdmap)", true);
    parser.add_argument("-a", "--allocator", "Allocator to use (builtin, optimal, nonoptimal)", true);
    parser.add_argument("-l", "--largepages", "Use large pages for custom allocator", false );
    
    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }

    if (parser.exists("t")) {
        std::string tree_type = parser.get<std::string>("t");
        std::cout << "Tree type : " << tree_type << std::endl;
        if (tree_type == "bfs") {
            out_tree_type = tree_type_e::BFS_TREE;
        } else if (tree_type == "dfs") {
            out_tree_type = tree_type_e::DFS_TREE;
        } else if (tree_type == "stdset") {
            out_tree_type = tree_type_e::STD_SET;
        } else {
            std::cout << "Unknown value for --tree\n";
            return false;
        }
    } else {
        return false;
    }

    if (parser.exists("a")) {
        std::string allocator_type = parser.get<std::string>("a");
        std::cout << "Allocator type : " << allocator_type << std::endl;
        if (allocator_type == "builtin") {
            out_allocator_type = allocator_type_e::BUILTIN;
        } else if (allocator_type == "optimal") {
            out_allocator_type = allocator_type_e::CUSTOM_OPTIMAL;
        } else if (allocator_type == "nonoptimal") {
            out_allocator_type = allocator_type_e::CUSTOM_NONOPTIMAL;
        } else {
            std::cout << "Unknown value for --allocator\n";
            return false;
        }
    } else {
        return false;
    }

    if (parser.exists("l")) {
        out_large_pages = parser.get<bool>("l");
    } else {
        out_large_pages = false;
    }
    std::cout << "Large pages " << out_large_pages << std::endl;

    return true;
}

template<typename allocator>
void measure_stdset(int arr_len) {
    std::vector<int> random_data_vector = create_random_array<int>(arr_len, 0, arr_len);
    std::set<int, std::less<int>, allocator> my_set(random_data_vector.begin(), random_data_vector.end());

    {
        measure_time m  ("std::set");
        {
            int count_found = 0;
            for (int i = 0; i < arr_len; i++) {
                if (my_set.find(random_data_vector[i]) != my_set.end()) {
                    count_found++;
                }
            }
            std::cout << "Count found = " << count_found << std::endl;
        }

    }
}

template
<typename allocator>
void measure_custom(int arr_len, tree_type_e tree_type) {
    std::vector<int> random_data_vector = create_random_array<int>(arr_len, 0, arr_len);
    std::vector<int> random_data_sorted(random_data_vector);
    std::sort(random_data_sorted.begin(), random_data_sorted.end());

    binary_search_tree<int, allocator> bst = tree_type == tree_type_e::BFS_TREE ? 
        binary_search_tree<int, allocator>::create_from_sorted_array_bfs(&random_data_sorted[0], arr_len) :
        binary_search_tree<int, allocator>::create_from_sorted_array_dfs(&random_data_sorted[0], arr_len);
    {
        measure_time m  ("BFS array");
        {
            int count_found = 0;
            for (int i = 0; i < arr_len; i++) {
                if (bst.find(random_data_vector[i])) {
                    count_found++;
                }
            }
            std::cout << "Count found = " << count_found << std::endl;
        }

    }
}

int main(int argc, const char* argv[]) {
    constexpr int arr_len = 10000000;

    std::cout << "Array length " << arr_len << std::endl;

    allocator_type_e allocator_type;
    tree_type_e tree_type;
    bool large_pages = false;

    if (!parse_args(argc, argv, tree_type, allocator_type, large_pages)) {
        std::cout << "Error parsing arguments" << std::endl;
        return -1;
    }

    zone_allocator_config<void>::set_use_large_pages(large_pages);

    if (tree_type == tree_type_e::BFS_TREE || tree_type == tree_type_e::DFS_TREE) {
        if (allocator_type == allocator_type_e::BUILTIN) {
            measure_custom<std::allocator<int>>(arr_len,tree_type);
        } else if (allocator_type == allocator_type_e::CUSTOM_OPTIMAL) {
            measure_custom<zone_allocator<int, 0>>(arr_len, tree_type);
        } else if (allocator_type == allocator_type_e::CUSTOM_NONOPTIMAL) {
            measure_custom<zone_allocator<int, 50>>(arr_len, tree_type);
        }

    } else {
        if (allocator_type == allocator_type_e::BUILTIN) {
            measure_stdset<std::allocator<int>>(arr_len);
        } else if (allocator_type == allocator_type_e::CUSTOM_OPTIMAL) {
            measure_stdset<zone_allocator<int, 0>>(arr_len);
        } else if (allocator_type == allocator_type_e::CUSTOM_NONOPTIMAL) {
            measure_stdset<zone_allocator<int, 50>>(arr_len);
        }
    }
}
