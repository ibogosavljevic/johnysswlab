#include <vector>
#include <cstdlib>
#include "utils.h"
#include "../common/Allocator.h"

template <typename T, typename NodeType, typename NodeStorage>
class binary_tree {
public:
    static binary_tree build_tree(const std::vector<T>& values) {
        binary_tree result;

        if (values.size() > 0) {
            result.root = result.build_tree_private(values, 0, values.size() - 1);
        } else {
            result.root = NodeType::null;
        }

        return result;
    }

    bool find(const T& value) {
        typename NodeType::node_id current_node = root;
        auto const base = NodeType::get_base(node_storage);
        
        while (current_node != NodeType::null) {
            NodeType* node = (NodeType*) (base + current_node);
            if (value < node->value) {
                current_node = node->left;
            } else if (value > node->value) {
                current_node = node->right;
            } else {
                return true;
            }
        }

        return false;
    }

    std::vector<bool> find_all(const std::vector<T>& vec) {
        std::vector<bool> result;
        result.reserve(vec.size());
        int size = vec.size();

        for (int i = 0; i < size; i++) {
            result.emplace_back(find(vec[i]));
        }

        return result;
    }

    std::vector<bool> find_all_interleaved(const std::vector<T>& vec) {
        int size = vec.size();
        int not_null_count;
        std::vector<typename NodeType::node_id> current_nodes(size, root);
        std::vector<bool> result(size, false);
        auto const base = NodeType::get_base(node_storage);

        do {
            not_null_count = 0;
            for (int i = 0; i < size; i++) {
                if (current_nodes[i] != NodeType::null) {
                    NodeType* node = (NodeType*) (base + current_nodes[i]);
                    if (vec[i] < node->value) {
                        current_nodes[i] = node->left;
                    } else if (vec[i] > node->value) {
                        current_nodes[i] = node->right;
                    } else {
                        current_nodes[i] = NodeType::null;
                        result[i] = true;
                    }

                    not_null_count++;
                }
            }
        } while (not_null_count > 0);

        return result;
    }

    static size_t get_size_of_chunk() {
        return sizeof(NodeType);
    }

private:
    typename NodeType::node_id build_tree_private(const std::vector<T>& values, size_t left, size_t right) {
        if (left > right) {
            return NodeType::null;
        } else {
            size_t middle = (right + left) / 2;
            typename NodeType::node_id node_id = NodeType::alloc(values[middle], node_storage);
            typename NodeType::node_id left_subtree;
            typename NodeType::node_id right_subtree;

            if (middle > left) {
                left_subtree = build_tree_private(values, left, middle - 1);
            } else {
                left_subtree = NodeType::null;
            }

            if (right > middle) {
                right_subtree = build_tree_private(values, middle + 1, right);
            } else {
                right_subtree = NodeType::null;
            }
                        
            NodeType* node = (NodeType*) (NodeType::get_base(node_storage) + node_id);
            node->left = left_subtree;
            node->right = right_subtree;

            return node_id;
        }
    }


    NodeStorage node_storage;
    typename NodeType::node_id root;
};

template <typename T>
struct simple_binary_tree_node {
    using node_id = simple_binary_tree_node*;

    template <typename NodeStorage>
    static node_id alloc(T val, NodeStorage& ns) { 
        simple_binary_tree_node* node = ns.allocate();
        node->value = val;
        return node;
    }

    static constexpr node_id null = 0;

    template <typename NodeStorage>
    static unsigned int get_base(NodeStorage& ns) {
        return 0;
    }

    T value;
    simple_binary_tree_node* left;
    simple_binary_tree_node* right;
};

template <typename T>
struct vector_backed_binary_tree_node {
    using node_id = unsigned int;

    template <typename NodeStorage>
    static node_id alloc(T val, NodeStorage& ns) {
        ns.emplace_back();
        node_id index = ns.size() - 1;
        ns[index].value = val;
        return index;
    }

    static constexpr node_id null = -1;

    template <typename NodeStorage>
    static vector_backed_binary_tree_node* get_base(NodeStorage& ns) {
        return &ns[0];
    }

    T value;
    unsigned int left;
    unsigned int right;
};

class dummy_storage {
};

