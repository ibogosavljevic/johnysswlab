#include <vector>


template <typename T>
class array_binary_tree {
public:
    static array_binary_tree build_tree(const std::vector<T>& values, T null_value) {
        array_binary_tree result(calculate_array_size(values.size()), null_value);

        if (values.size() > 0) {
            result.m_root = result.build_tree_private(values, 0, values.size() - 1, 0);
        } else {
            result.m_root = nullptr;
        }

        return result;
    }

    bool find(const T& value) {
        node* current_node = m_root;

        while(current_node != nullptr) {
            if (value < current_node->value) {
                current_node = current_node->left;
            } else if (value > current_node->value) {
                current_node = current_node->right;
            } else {
                return true;
            }
        }

        return false;
    }

    bool find_array(const T& value) {
        int current_index = 0;
        int size = m_nodes.size();

        while (current_index < size) {
            T current_value = m_nodes[current_index].value;
            int new_index = 2 * current_index + 1;
            if (value > current_value) {
                new_index++;
            } else if (value == current_value) {
                return true;
            }
            current_index = new_index;
        }

        return false;
    }

private:
    array_binary_tree(int size, T null_value) : m_nodes(size, node(null_value)), m_null_value(null_value)  {}

    static int calculate_array_size(int c) {
        int current_increment = 1;
        int current_size = 0;

        while (c > current_size) {
            current_size += current_increment;
            current_increment *= 2;
        }

        return current_size;
    }

    struct node {
        T value;
        node* left;
        node* right;

        node(T v) : value(v), left(nullptr), right(nullptr) {}
    };

    node* build_tree_private(const std::vector<T>& values, int left, int right, int index) {
        if (left > right) {
            return nullptr;
        } else {
            size_t middle = (right + left) / 2;
            
            node* left_subtree;
            node* right_subtree;

            if (middle > left) {
                left_subtree = build_tree_private(values, left, middle - 1, 2*index + 1);
            } else {
                left_subtree = nullptr;
            }

            if (right > middle) {
                right_subtree = build_tree_private(values, middle + 1, right, 2*index + 2);
            } else {
                right_subtree = nullptr;
            }
                        
            m_nodes[index].value = values[middle];
            m_nodes[index].left = left_subtree;
            m_nodes[index].right = right_subtree;

            return &m_nodes[index];
        }

    }

    std::vector<node> m_nodes;
    node* m_root;
    T m_null_value;
};