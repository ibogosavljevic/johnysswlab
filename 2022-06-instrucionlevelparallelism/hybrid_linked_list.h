#include <vector>
#include <cassert>

template <typename T>
class hybrid_linked_list {
public:
    hybrid_linked_list(int max_size) {
        m_nodes = new node[max_size];
        m_head = nullptr;
        m_tail = nullptr;
        m_end = m_nodes + max_size;
        m_free = m_nodes;
    }

    ~hybrid_linked_list() {
        if (m_nodes) {
            delete [] m_nodes;
        }

        m_nodes = nullptr;
        m_head = nullptr;
        m_end = nullptr;
        m_free = nullptr;
        m_tail = nullptr;
    }

    bool push_back(const T& val) {
        if (m_free >= m_end) {
            return false;
        }
        
        m_free->value = val;
        m_free->next = nullptr;

        if (m_head == nullptr) {
            m_head = m_free;
            assert(m_tail == nullptr);
        } else {
            m_tail->next = m_free;
        }

        m_tail = m_free;
        m_free++;
        
        return true;
    }

    template <typename Q>
    void delete_simple(Q begin, Q end) {
        for (auto it = begin; it != end; ++it) {
            node* current = m_head;
            node* prev = nullptr;
            T val = *it;

            while (current != nullptr) {
                if (current->value == val) {
                    if (prev != nullptr) {
                        prev->next = current->next;
                    } else {
                        m_head = current->next;
                    }

                    current->value.~T();
                    current->next = UINTPTR_MAX;
                }

                current = current->next;
            }
        }
    }

    template <typename Q>
    void delete_interleaved(Q begin, Q end) {
        node* current = m_head;
        node* prev = nullptr;

        while (current != nullptr) {
            T current_val = current->value;

            for (auto it = begin; it != end; ++it) {
                if (*it == current_val) {
                    if (prev != nullptr) {
                        prev->next = current->next;
                    } else {
                        m_head = current->next;
                    }
                    current->value.~T();
                    current->next = UINTPTR_MAX;
                    break;
                }
            }

            current = current->next;
        }
    }

    std::vector<bool> lookup_values_simple(std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        int size = values.size();

        for (int i = 0; i < size; i++) {
            T val = values[i];
            node* current_node = m_head;

            while (current_node != nullptr) {
                if (current_node->value == val) {
                    result[i] = true;
                    break;
                }

                current_node = current_node->next;
            }
        }

        return result;
    }

    std::vector<bool> lookup_values_interleaved(std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        int size = values.size();

        node* current_node = m_head;
        while (current_node != nullptr) {
            T current_value = current_node->value;
            for (int i = 0; i < size; i++) {
                if (current_value == values[i]) {
                    result[i] = true;
                    break;
                }
            }

            current_node = current_node->next;
        }

        return result;
    }

    std::vector<bool> lookup_values_array(std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        
        for (int i = 0; i < values.size(); i++) {
            for (node* n = m_nodes; n < m_free; n++) {
                if (n->next != reinterpret_cast<node*>(UINTPTR_MAX)) {
                    if (n->value == values[i]) {
                        result[i] = true;
                        break;
                    }
                }
            }
        }

        return result;
    }

    std::vector<bool> lookup_values_index(std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        std::vector<node*> index_vector;

        node* current_node = m_head;
        while (current_node != nullptr) {
            index_vector.push_back(current_node);
            current_node = current_node->next;
        }
        
        for (int i = 0; i < values.size(); i++) {
            for (int j = 0; j < index_vector.size(); j++) {
                if (index_vector[j]->value == values[i]) {
                    result[i] = true;
                    break;
                }
            }
        }

        return result;
    }

    void shuffle_list(int count) {
        std::vector<node*> index_vector;

        node* current_node = m_head;
        while (current_node != nullptr) {
            index_vector.push_back(current_node);
            current_node = current_node->next;
        }

        if (count > index_vector.size()) {
            count = index_vector.size();
        }

        std::vector<int> indexes(index_vector.size());
        std::iota(indexes.begin(), indexes.end(), 0);
        std::random_shuffle(indexes.begin(), indexes.end());

        for (int i = 0; i < count; i++) {
            // Pick a random element between 0 and index_vector.size() - 1
            int index = indexes[i];

            // Exchange the place of element 0 and the random element
            if (index > 0) {
                node* zero = index_vector[0];
                node* random = index_vector[index];
                node* prev_random = index_vector[index - 1];
                node* zero_next = index_vector[1];

                zero->next = random->next;
                prev_random->next = zero;

                random->next = zero_next;
                m_head = random;
                
                if (m_tail == random) {
                    m_tail = zero;
                }
            }

            std::swap(index_vector[0], index_vector[index]);
        }
    }

private:
    struct node {
        T value;
        node* next;
    };

    node* m_nodes;
    node* m_head;
    node* m_tail;
    node* m_end;
    node* m_free;
    
};