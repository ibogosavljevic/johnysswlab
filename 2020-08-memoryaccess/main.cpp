#include <set>
#include "custom_allocator.h"
#include "utils.h"
#include "measure_time.h"



int main() {
    constexpr int arr_len = 10000000;
    std::vector<int> random_data_vector = create_random_array<int>(arr_len, 0, arr_len);
    std::set<int, std::less<int>, zone_allocator<int, 0>> my_set(random_data_vector.begin(), random_data_vector.end());

    {
        measure_time m  ("Custom allocator");
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