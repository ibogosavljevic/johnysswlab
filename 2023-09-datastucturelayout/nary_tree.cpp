#include <array>
#include <cmath>
#include <vector>
#include <limits>
#include <iostream>
#include <cassert>
#include <numeric>
#include <algorithm>

template <typename T, int node_count = 1>
class search_tree {
public:
    template <typename It>
    search_tree(It start, It end) : m_nodes((end - start) + 1), m_nodes_index((end - start) + 1) {
        size_t len = end - start;

        allocate_initialize();
        
        m_start_node = create_tree(start, 0, len - 1);
    }

    void dump_tree(std::ostream& os) {
        os << "Start node " << m_start_node << std::endl;
        for (int i = 1; i < m_nodes.size(); i++) {
            os << "Node " << i << ": ";
            size_t count = m_nodes[i].m_count;
            for (int j = 0; j < count; j++) {
                os << "(" << m_nodes[i].m_next[j] << ") ";
                os << *m_nodes[i].value(j) << " ";
            }
            os << "(" << m_nodes[i].m_next[count] << ")\n";
        }
    }

    bool find(const T& value) const {
        bool found = false;

        const node_t* current = &m_nodes[m_start_node];

        while (true) {
            if (value <= current->first_value()) {
                if (value == current->first_value()) {
                    found = true;
                    goto done;
                } else {
                    auto next_node = current->first_next();
                    if (next_node != NULL_NODE) {
                        current = &m_nodes[next_node];
                    } else {
                        found = false;
                        goto done;
                    }
                }
            } else if (value >= current->last_value()) {
                if (value == current->last_value()) {
                    found = true;
                    goto done;
                } else {
                auto next_node = current->last_next();
                    if (next_node != NULL_NODE) {
                        current = &m_nodes[next_node];
                    } else {
                        found = false;
                        goto done;
                    }
                }
            } else {
                for (int i = 0; i < current->m_count - 1; i++) {
                    const T& val_left = current->value_at(i);
                    const T& val_right = current->value_at(i+1);
                    if (value == val_left) {
                        found = true;
                        goto done;
                    } else if (value == val_right) {
                        found = true;
                        goto done;
                    } else if (value > val_left & value < val_right) {
                        auto next_node = current->m_next[i + 1];
                        if (next_node != NULL_NODE) {
                            current = &m_nodes[next_node];
                        } else {
                            found = false;
                            goto done;
                        }
                    }
                }
            }
        }
done:
        return found;
    }

    void print(std::ostream& os) {
        os << "size of struct = " << sizeof(node_t) << "\n"; 
        os << "size of m_values = " << sizeof(m_nodes[0].m_values) << "\n";
        os << "size of m_next = " << sizeof(m_nodes[0].m_next) << "\n"; 
    }

private:
    static const uint32_t MAX_NODE = std::numeric_limits<uint32_t>::max();
    static const uint32_t NULL_NODE = 0;

    struct /*alignas(64)*/ node_t {
        std::array<char[sizeof(T)], node_count> m_values;
        uint32_t m_next[node_count + 1];
        uint32_t m_count;

        T* value(int i) {
            return reinterpret_cast<T*>(m_values[i]);
        }

        const T& value_at(int i) const { 
            return *reinterpret_cast<const T*>(m_values[i]);
        }

        const T& first_value() const {
            return value_at(0);
        }

        const T& last_value() const {
            return value_at(m_count - 1);
        }

        uint32_t first_next() const { return m_next[0]; }
        uint32_t last_next() const { return m_next[m_count]; }

    };

    std::vector<node_t> m_nodes;
    std::vector<size_t> m_nodes_index;
    uint32_t m_free_node;
    uint32_t m_start_node;

    template <typename It>
    uint32_t create_tree(It& a, size_t start, size_t end) {
        uint32_t result;

        if (start > end) {
            return 0;
        }

        uint32_t len = end - start + 1;

        if (len <= node_count) {
            result = allocate_node();
            node_t& node = m_nodes[result];
            
            node.m_count = len;
            for (int i = 0; i < len; i++) {
                new (node.value(i)) T(a[start + i]);
                node.m_next[i] = NULL_NODE;
            }

        } 
        else {
            size_t current_index;
            double stride = len / static_cast<double>(node_count + 1);

            result = allocate_node();
            node_t& node = m_nodes[result];

            size_t previous_index = start - 1;
            node.m_count = node_count;
            for (int i = 0; i < node_count; i++) {
                current_index = start + static_cast<size_t>((i + 1) * stride + 0.5);

                new (node.value(i)) T(a[current_index]);

                size_t new_index_left = previous_index + 1;
                size_t new_index_right = current_index - 1;

                if (new_index_right >= new_index_left) {
                    node.m_next[i] = create_tree(a, new_index_left, new_index_right);
                } else {
                    node.m_next[i] = NULL_NODE;
                }

                previous_index = current_index;
            }

            size_t new_index_left = current_index + 1;

            if (end >= new_index_left) {
                node.m_next[node_count] = create_tree(a, new_index_left, end);
            } else {
                node.m_next[node_count] = NULL_NODE;
            }
        }

        return result;
    }

    void allocate_initialize() {
        // First node in list is reserved
        m_nodes.emplace_back();
        m_free_node = 1;

        std::iota(m_nodes_index.begin(), m_nodes_index.end(), 1);
        std::random_shuffle(m_nodes_index.begin(), m_nodes_index.end());
    }

    uint32_t allocate_node() {
        uint32_t free_node = m_free_node;
        m_free_node++;
        return m_nodes_index.at(free_node);
    }

};

#include <algorithm>
#include "likwid.h"
#include "utils.h"


static constexpr size_t ARR_LEN = 10* 1024 * 1024;

template <int len>
void measure_time(std::string name, const std::vector<int>& input, const std::vector<int>& search_data) {
    search_tree<int, len> s(input.begin(), input.end());

    std::cout << name << ", measure time " << len << std::endl;
    s.print(std::cout);
    //s.dump_tree(std::cout);

    int start_value = input[0] - 5;
    int end_value = input[input.size() - 1] + 5;

    int found = 0;

    {
        LIKWID_MARKER_START(name.c_str());
        for (int i: search_data) {
            bool result = s.find(i);
            found += result;
        }
        LIKWID_MARKER_STOP(name.c_str());
    }

    std::cout << "Found " << found << std::endl;
}

int main(int argc, char** argv) {
    static constexpr size_t lookup_count = 10*1024*1024;

    static constexpr size_t min_size = 4*1024;
    static constexpr size_t max_size = 16*1024*1024;

    LIKWID_MARKER_INIT;

    for (size_t size = min_size; size <= max_size; size *= 4) {
        std::vector<int> sorted_data = create_random_array<int>(size, 0, size*2);
        std::sort(sorted_data.begin(), sorted_data.end());
        auto it = std::unique(sorted_data.begin(), sorted_data.end());
        sorted_data.resize(std::distance(sorted_data.begin(), it));

        std::vector<int> search_data = create_random_array<int>(lookup_count, -10, size*2 + 10);
        std::string name = "size_" + std::to_string(size) + "_";

        measure_time<1>(name + "1", sorted_data, search_data);
        measure_time<2>(name + "2", sorted_data, search_data);
        measure_time<3>(name + "3", sorted_data, search_data);
        measure_time<4>(name + "4", sorted_data, search_data);
        measure_time<5>(name + "5", sorted_data, search_data);
        measure_time<6>(name + "6", sorted_data, search_data);
        measure_time<7>(name + "7", sorted_data, search_data);
        measure_time<8>(name + "8", sorted_data, search_data);
    }

    LIKWID_MARKER_CLOSE;

}