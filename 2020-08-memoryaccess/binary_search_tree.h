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

        std::queue<bsc_bfs> bsc_queue;
        bs_chunk* head;

        bsc_queue.emplace(&head, 0, len - 1);

        while(!bsc_queue.empty()) {
            int left = bsc_queue.front().m_left;
            int right = bsc_queue.front().m_right;

            int middle = (left + right) / 2;

            bs_chunk** parent = bsc_queue.front().m_parent;
            bs_chunk* c = new bs_chunk(array[middle]);

            *parent = c;

            if (left <= middle - 1) {
                bsc_queue.emplace(&c->m_left, left, middle - 1);
            }

            if (middle + 1 <= right) {
                bsc_queue.emplace(&c->m_right, middle + 1, right);
            }

            bsc_queue.pop();
        }

        return binary_search_tree(head);
        
    }

    static binary_search_tree create_from_sorted_array_dfs(T* array, int len) {
        bs_chunk* head = create_dfs(array, 0, len - 1);

        return binary_search_tree(head);
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

    bs_chunk* m_head;

    static bs_chunk* create_dfs(T* array, int left, int right) {
        
        if (left > right) {
            return nullptr;
        }
        
        int middle = (left + right) / 2;

        bs_chunk * b = new bs_chunk(array[middle]);
        b->m_left = create_dfs(array, left, middle - 1);
        b->m_right = create_dfs(array, middle + 1, right);

        return b;
    }

    void destroy(bs_chunk* head) {
        if (head->m_left) {
            destroy(head->m_left);
        }

        if (head->m_right) {
            destroy(head->m_right);
        }

        delete head;
    }

    binary_search_tree(bs_chunk* head) :
        m_head(head) {}

};
