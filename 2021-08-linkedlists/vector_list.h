#include <cstdint>
#include <limits>
#include <vector>

namespace jsl {

template <typename T>
class vector_list {
   public:
    typedef uint32_t size_type;
    typedef uint32_t iterator_t;

    vector_list()
        : m_start_index(INVALID_INDEX),
          m_free_index(INVALID_INDEX),
          m_size(0) {}

    vector_list(size_type reserve_count)
        : m_start_index(INVALID_INDEX), m_free_index(INVALID_INDEX), m_size(0) {
        m_vector.reserve(reserve_count);
    }

    void reserve(size_type n) { m_vector.reserve(n); }

    iterator_t begin() { return m_start_index; }

    iterator_t end() { return INVALID_INDEX; }

    iterator_t next(iterator_t current) { return m_vector[current].next; }

    iterator_t insert_after(iterator_t current, const T& value) {
        iterator_t node = allocate_node();

        new (m_vector[node].value) T(value);
        m_vector[node].next = m_vector[current].next;
        m_vector[current].next = node;

        m_size++;

        return node;
    }

    iterator_t erase_after(iterator_t current) {
        if (m_vector[current].next != INVALID_INDEX) {
            iterator_t to_delete = m_vector[current].next;
            iterator_t return_iterator = m_vector[m_vector[current].next].next;
            m_vector[current].next = return_iterator;
            at(to_delete).~T();
            release_node(to_delete);

            m_size--;
            return return_iterator;
        }

        return INVALID_INDEX;
    }

    void push_front(const T& val) {
        iterator_t node = allocate_node();

        new (m_vector[node].value) T(val);
        m_vector[node].next = m_start_index;
        m_start_index = node;

        m_size++;
    }

    void pop_front() {
        if (m_start_index != INVALID_INDEX) {
            at(m_start_index).~T();
            int to_delete = m_start_index;
            m_start_index = m_vector[m_start_index].next;
            release_node(to_delete);
            m_size--;
        }
    }

    T& front() { return at(m_start_index); }

    T& at(iterator_t current) {
        return *(reinterpret_cast<T*>(m_vector[current].value));
    }

    void compact() {
        std::vector<node> new_vector(m_size);

        int i = 0;
        for (auto it = begin(); it != end(); it = next(it)) {
            new (new_vector[i].value) T(std::move(at(it)));

            at(it).~T();
            i++;
            new_vector[i - 1].next = i;
        }

        new_vector[i - 1].next = INVALID_INDEX;

        m_vector = std::move(new_vector);
        m_start_index = 0;
        m_free_index = INVALID_INDEX;
    }

   private:
    struct node {
        char value[sizeof(T)];
        iterator_t next;
    };

    iterator_t allocate_node() {
        if (m_free_index != INVALID_INDEX) {
            iterator_t new_node = m_free_index;
            m_free_index = next(m_free_index);
            return new_node;
        } else {
            m_vector.emplace_back();
            return m_vector.size() - 1;
        }
    }

    void release_node(iterator_t node) {
        m_vector[node].next = m_free_index;
        m_free_index = node;
    }

    std::vector<node> m_vector;

    static constexpr iterator_t INVALID_INDEX =
        std::numeric_limits<iterator_t>::max();

    iterator_t m_start_index;
    iterator_t m_free_index;
    iterator_t m_size;
};

}  // namespace jsl