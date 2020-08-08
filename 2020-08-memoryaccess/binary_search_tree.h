#include <queue>

template <typename T, typename Alloc>
class binary_search_tree {
public:
    static binary_search_tree create_from_sorted_array_bfs(T* array, int len) {
        struct bsc_bfs {
            int m_left;
            int m_right;
            bs_chunk** m_parent;

            bsc_bfs(bs_chunk** parent, int left, int right): 
                m_parent(parent),
                m_left(left),
                m_right(right) { }
        };

        binary_search_tree result(nullptr);

        std::queue<bsc_bfs> bsc_queue;
        bs_chunk* head = nullptr;

        bsc_queue.emplace(&head, 0, len - 1);

        while(!bsc_queue.empty()) {
            int left = bsc_queue.front().m_left;
            int right = bsc_queue.front().m_right;

            int middle = (left + right) / 2;

            bs_chunk** parent = bsc_queue.front().m_parent;
            bs_chunk* c = result.m_allocator.allocate(1);
            ::new (c) bs_chunk(array[middle]);

            *parent = c;

            if (left <= middle - 1) {
                bsc_queue.emplace(&c->m_left, left, middle - 1);
            }

            if (middle + 1 <= right) {
                bsc_queue.emplace(&c->m_right, middle + 1, right);
            }

            bsc_queue.pop();
        }

        result.m_head = head;
        return result;
        
    }

    static binary_search_tree create_from_sorted_array_dfs(T* array, int len) {
        binary_search_tree result(nullptr);
        
        result.m_head = create_dfs(result, array, 0, len - 1);
        return result;
    }

    bool find(const T& value) {
        bs_chunk* current = m_head;

        while (current) {
            if (current->m_value == value) {
                return true;
            }

            if (value < current->m_value) {
                current = current->m_left;
            } else {
                current = current->m_right;
            }
        }

        return false;
    }

    ~binary_search_tree() {
        if (m_head) {
            destroy(m_head);
        }
    }

private:
    struct bs_chunk {
        T m_value;
        bs_chunk* m_left;
        bs_chunk* m_right;

        bs_chunk(T value) : m_value(value), m_left(nullptr), m_right(nullptr) {}
    };

    using node_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<bs_chunk>;


    bs_chunk* m_head;
    node_alloc_t m_allocator;

    static bs_chunk* create_dfs(binary_search_tree& bt, T* array, int left, int right) {
        
        if (left > right) {
            return nullptr;
        }
        
        int middle = (left + right) / 2;

        bs_chunk* b = bt.m_allocator.allocate(1);
        ::new (b) bs_chunk(array[middle]);

        b->m_left = create_dfs(bt, array, left, middle - 1);
        b->m_right = create_dfs(bt, array, middle + 1, right);

        return b;
    }

    void destroy(bs_chunk* head) {
        if (head->m_left) {
            destroy(head->m_left);
        }

        if (head->m_right) {
            destroy(head->m_right);
        }

        head->~bs_chunk();
        m_allocator.deallocate(head, 1);
    }

    binary_search_tree(bs_chunk* head) :
        m_head(head) {}

};
