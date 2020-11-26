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

    static binary_search_tree create_from_sorted_array_dfs_preorder(T* array, int len) {
        binary_search_tree result(nullptr);
        
        result.m_head = create_dfs_preorder(result, array, 0, len - 1);
        return result;
    }

    static binary_search_tree create_from_sorted_array_dfs_inorder(T* array, int len) {
        binary_search_tree result(nullptr);
        
        result.m_head = create_dfs_inorder(result, array, 0, len - 1);
        return result;
    }

    static binary_search_tree create_from_sorted_array_van_emde_boas_layout(T* array, int len) {
        binary_search_tree result(nullptr);

        result.m_head = create_van_emde_boas_layout(result, array, 0, len - 1);
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

    void dump_tree() {
        std::queue<bs_chunk*> bfs_queue;
        std::vector<bs_chunk*> bfs_order;

        bfs_queue.push(m_head);
        while (!bfs_queue.empty()) {
            bs_chunk* c = bfs_queue.front();
            bfs_order.push_back(c);

            if (c->m_left) {
                bfs_queue.push(c->m_left);
            }

            if (c->m_right) {
                bfs_queue.push(c->m_right);
            }

            bfs_queue.pop();
        }

        for(bs_chunk* c: bfs_order) {
            std::cout << "Value = " << c->m_value;
            if (c->m_left) {
                std::cout << ", left = " << c->m_left->m_value;
            }

            if (c->m_right) {
                std::cout << ", right = " << c->m_right->m_value;
            }

            std::cout << std::endl;
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

    static bs_chunk* create_dfs_preorder(binary_search_tree& bt, T* array, int left, int right) {
        
        if (left > right) {
            return nullptr;
        }
        
        int middle = (left + right) / 2;

        bs_chunk* b = bt.m_allocator.allocate(1);
        ::new (b) bs_chunk(array[middle]);

        b->m_left = create_dfs_preorder(bt, array, left, middle - 1);
        b->m_right = create_dfs_preorder(bt, array, middle + 1, right);

        return b;
    }

    static bs_chunk* create_van_emde_boas_layout(binary_search_tree& bt, T* array, int left, int right) {
        if (left > right) {
            return nullptr;
        }

        int middle = (left + right) / 2;
        int left_middle = (left + middle) / 2;
        int right_middle = (right + middle) / 2;
        bool left_exists = false;
        bool right_exists = false;
        bs_chunk* bl;
        bs_chunk* br;

        //std::cout << "left = " << array[left] << ", left_middle = " << array[left_middle];
        //std::cout << ", middle = " << array[middle];
        //std::cout  << ", right_middle = " << array[right_middle] << ", right = " << array[right] << std::endl;

        bs_chunk* b = bt.m_allocator.allocate(1);
        ::new (b) bs_chunk(array[middle]);


        if (left_middle < middle) {
            bl = bt.m_allocator.allocate(1);
            ::new (bl) bs_chunk(array[left_middle]);
            b->m_left = bl;
            left_exists = true;
        } else {
            b->m_left = nullptr;
        }

        if (middle < right_middle) {
            br = bt.m_allocator.allocate(1);
            ::new (br) bs_chunk(array[right_middle]);
            b->m_right = br;
            right_exists = true;
        } else {
            b->m_right = nullptr;
        }

        if (left_exists) {
            bl->m_left = create_van_emde_boas_layout(bt, array, left, left_middle - 1);
            bl->m_right = create_van_emde_boas_layout(bt, array, left_middle + 1, middle - 1);
        } else {
            b->m_left = create_van_emde_boas_layout(bt, array, left, middle - 1);
        }

        if (right_exists) {
            br->m_left = create_van_emde_boas_layout(bt, array, middle + 1, right_middle - 1);
            br->m_right = create_van_emde_boas_layout(bt, array, right_middle + 1, right);
        } else {
            b->m_right = create_van_emde_boas_layout(bt, array, middle + 1, right);
        }

        return b;
    }

    static bs_chunk* create_dfs_inorder(binary_search_tree& bt, T* array, int left, int right) {
        
        if (left > right) {
            return nullptr;
        }
        
        int middle = (left + right) / 2;

        bs_chunk* tmp = create_dfs_inorder(bt, array, left, middle - 1);

        bs_chunk* b = bt.m_allocator.allocate(1);
        ::new (b) bs_chunk(array[middle]);
        b->m_left = tmp;

        b->m_right = create_dfs_inorder(bt, array, middle + 1, right);

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
