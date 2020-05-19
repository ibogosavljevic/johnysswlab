
#include <cassert>


template<typename T, int count>
class linked_list {
private:

    class linked_list_node {
    public:
#ifdef OPTIMAL
        char used_elems[count];
        linked_list_node* next;
        char values[count * sizeof(T)];
#elif defined(SUBOPTIMAL)
        linked_list_node* next;
        char values[count * sizeof(T)];
        char used_elems[count];
#else
        #error Need to define OPTIMAL or SUBOPTIMAL
#endif

        template<typename ...Args>
        void create(const int pos, const Args &... args) {
            assert(used_elems[pos] == 0);
            ::new (&values[pos * sizeof(T)]) T(args...);
            used_elems[pos] = 1;
        }

        void destroy(const int pos) {
            to_reference(pos).~T();
            used_elems[pos] = 0;
        }

        T& to_reference(const int pos) {
            return *to_pointer(pos);
        }

        const T& to_const_reference(const int pos) {
            return *to_const_pointer(pos);
        }

        T* to_pointer(const int pos) {
            return (T*) &(values[pos * sizeof(T)]);
        }

        const T* to_const_pointer(const int pos) {
            return (const T*) & (values[pos * sizeof(T)]);
        }

        void get_next(int pos, linked_list_node*& next_elem, int next_pos) {
            for (int i = pos; i < count; i++) {
                if (used_elems[i] != 0) {
                    next_elem = this;
                    next_pos = i;
                    return;
                }
            }

            next_elem = next;
            if (next_elem == nullptr) {
                next_pos = 0;
                return;
            }

            for (int i = 0; i < count; i++) {
                if (next_elem->used_elems[i] != 0) {
                    next_pos = i;
                    return;
                }
            }

            assert(false);
        }

        bool used(const int pos) {
            return used_elems[pos];
        }

    };

    linked_list_node* begin;
    linked_list_node* end;

public:
    typedef int size_type;

    linked_list() {
        begin = new linked_list_node();
        end = begin;
    }

    template<typename ...Args>
    void emplace_back(const Args &... args) {
        int last_free_pos = -1;
        for (int i = count - 1; i >= 0; i--) {
            if (end->used(i) == 0) {
                last_free_pos = i;
            } else {
                break;
            }
        }

        if (last_free_pos == -1) {
            linked_list_node* new_elem = new linked_list_node();
            new_elem->create(0, args...);
            end->next = new_elem;
            end = new_elem;
            return;
        }

        assert(last_free_pos >= 0 && last_free_pos < count);
        end->create(last_free_pos, args...);
    }

    template <typename Cond>
    int remove_if (Cond&& condition) {
        linked_list_node* current = begin;
        linked_list_node* next;
        linked_list_node* prev = nullptr;
        int removed = 0;
        bool removed_iteration;
        bool block_deleted;

        while (current != nullptr) {
            removed_iteration = false;
            block_deleted = false;
            for (int i = 0; i < count; i++) {
                if (current->used(i)) {
                    if (condition(current->to_const_reference(i))) {
                        current->destroy(i);
                        removed++;
                        removed_iteration = true;
                    }
                }
            }

            next = current->next;

            if (removed_iteration) {
                block_deleted = delete_block_if_empty(current, prev);
            }

            if (block_deleted) {
                // prev doesn't change
                current = next;
            } else {
                current = next;
                prev = current;
            }
        }
        return removed;
    }

    template <typename Cond>
    bool find_if(Cond&& condition) {
        linked_list_node* current = begin;

        while (current != nullptr) {
            for (int i = 0; i < count; i++) {
                if (current->used(i)) {
                    if (condition(current->to_const_reference(i))) {
                        return true;
                    }
                }
            }
            current = current->next;
        }

        return false;
    }

    bool dump() {
        linked_list_node* current = begin;
        while (current != nullptr) {
            
            for (int i = 0; i < count; i++) {
                if (current->used(i)) {
                    std::cout << current->to_const_reference(i) << "\n";
                }
            }
            current = current->next;
        }

        return false;
    }

    ~linked_list() {
        linked_list_node* current = begin;
        linked_list_node* tmp;
        while (current != nullptr) {
            
            for (int i = 0; i < count; i++) {
                if (current->used(i)) {
                    current->destroy(i);
                }
            }
            tmp = current;
            current = current->next;
            delete tmp;
        }
    }

private:
    bool delete_block_if_empty(linked_list_node* current, linked_list_node* prev) {
        bool empty_block = true;
        bool deleted;
        for (int i = 0; i < count; i++) {
            if (current->used(i)) {
                empty_block = false;
                break;
            }
        }

        if (empty_block) {
            if (prev) {
                prev->next = current->next;
            } else {
                assert(current == begin);
                begin = current->next;
            }
            delete current;
            deleted = true;
        } else {
            deleted = false;
        }

        return deleted;
    }
};