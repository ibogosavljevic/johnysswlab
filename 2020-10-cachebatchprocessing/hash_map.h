#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <functional>
#include <iostream>

namespace jsl {

struct simple_hasher {
    size_t m_limit;
    void set_limit(size_t limit) { m_limit = limit; }
    size_t limit_input(size_t val) { return val % m_limit; }
    size_t limit_input(size_t val, size_t limit) { return val % m_limit; }
};

struct shift_hasher {
    size_t m_limit;
    void set_limit(size_t limit) { m_limit = limit - 1; }
    size_t limit_input(size_t val) { return (val & m_limit); }
    size_t limit_input(size_t val, size_t limit) { return (val & (limit - 1)); }
};

template <typename T, typename F = shift_hasher>
class hash_map {
   public:
    hash_map(size_t capacity)
        : m_log_size(next_pow2_log(capacity)),
          m_size(1 << m_log_size),
          m_used(0),
          m_used_and_deleted(0),
          m_rehashing_threshhold(m_size * 0.7) {
        m_limiter.set_limit(m_size);
        int result = posix_memalign((void**)&m_values, 64, m_size * sizeof(T));
        if (result != 0) {
            throw std::bad_alloc();
        }
        m_value_used = (char*)malloc(m_size);

        std::memset(m_values, 0, m_size * sizeof(T));
        std::memset(m_value_used, 0, m_size);
    }

    ~hash_map() {
        free(m_value_used);
        free(m_values);
    }

    hash_map() : hash_map(DEFAULT_HASH_SIZE) {}

    template <bool calculate_hash = true>
    bool find(const T& val, size_t hash = 0) {
        size_t entry_start;

        if (calculate_hash) {
            entry_start = m_limiter.limit_input(m_hasher(val));
        } else {
            entry_start = hash;
        }

        size_t entry_current = entry_start;

        do {
            if (m_value_used[entry_current] == BUCKET_USED) {
                T* p = get(m_values, entry_current);
                if (*p == val) {
                    return true;
                } else {
                    entry_current++;
                }
            } else if (m_value_used[entry_current] == BUCKET_DELETED) {
                entry_current++;
            } else {
                return false;
            }

            if (entry_current == m_size) {
                entry_current = 0;
            }
        } while (entry_current != entry_start);

        return false;
    }

    template <int parallel_searches = 8>
    std::vector<bool> find_multiple(const std::vector<T>& in_values) {
        std::vector<bool> result(in_values.size(), false);
        std::array<find_single_t, parallel_searches> find_desc;

        // Round the length to lower four
        int len = (in_values.size() / parallel_searches) * parallel_searches;

        for (int i = 0; i < len; i += parallel_searches) {
            for (int j = 0; j < parallel_searches; j++) {
                size_t hash = m_limiter.limit_input(m_hasher(in_values[i + j]));
                __builtin_prefetch(&m_value_used[hash]);
                find_desc[j].init(hash);
            }

            for (size_t j = 0, index = i; j < parallel_searches; index++, j++) {
                find_desc[j].find_result =
                    find_single(in_values[index], find_desc[j]);
            }

            for (size_t j = 0, index = i; j < parallel_searches; index++, j++) {
                if (find_desc[j].find_result == find_single_result_e::RUNNING) {
                    find_desc[j].find_result =
                        find_single(in_values[index], find_desc[j]);
                }

                //                assert(find_desc[j].find_result ==
                //                find_single_result_e::FOUND ||
                //                        find_desc[j].find_result ==
                //                        find_single_result_e::NOT_FOUND);

                result[index] = find_desc[j].find_result;
            }
        }

        return result;
    }

    template <int look_ahead = 4>
    std::vector<bool> find(const std::vector<T>& in_values) {
        std::vector<bool> result(in_values.size(), false);
        std::array<size_t, look_ahead> hashes;
        size_t len = in_values.size();

        for (size_t i = 0; i < look_ahead; i++) {
            hashes[i] = m_limiter.limit_input(m_hasher(in_values[i]));
        }

        for (size_t i = 0; i < len; i++) {
            size_t look_ahaed_index = i + look_ahead;
            size_t hash_index =
                i %
                look_ahead;  // (i + look_ahead ) % look_ahead == i % look_ahead
            size_t my_hash = hashes[hash_index];
            size_t new_hash =
                m_limiter.limit_input(m_hasher(in_values[look_ahaed_index]));
            hashes[hash_index] = new_hash;

            __builtin_prefetch(&m_value_used[new_hash]);

            result[i] = find<false>(in_values[i], my_hash);
        }

        return result;
    }

    bool insert(const T& val) {
        size_t entry_start = m_limiter.limit_input(m_hasher(val));
        size_t entry_current = entry_start;

        do {
            if (m_value_used[entry_current] == BUCKET_USED) {
                if (*get(m_values, entry_current) == val) {
                    return false;
                } else {
                    entry_current++;
                    if (entry_current == m_size) {
                        entry_current = 0;
                    }
                }
            } else {
                break;
            }

        } while (entry_current != entry_start);

        if (m_value_used[entry_current] == BUCKET_FREE) {
            m_used++;
            m_used_and_deleted++;
        } else {
            // DELETED
            m_used++;
        }

        // DELETED or FREE bucket can be used
        ::new (get(m_values, entry_current)) T(val);
        m_value_used[entry_current] = BUCKET_USED;

        if (m_used_and_deleted > m_rehashing_threshhold) {
            if (m_used > (0.8 * m_rehashing_threshhold)) {
                grow_and_rehash(m_log_size + 1);
            } else {
                grow_and_rehash(m_log_size);
            }
        }

        return true;
    }

    bool remove(const T& val) {
        size_t entry_start = m_limiter.limit_input(m_hasher(val));
        size_t entry_current = entry_start;

        do {
            if (m_value_used[entry_current] == BUCKET_USED) {
                T* v = get(m_values, entry_current);
                if (*v == val) {
                    v->~T();
                    m_value_used[entry_current] = BUCKET_DELETED;
                    m_used--;
                    return true;
                } else {
                    entry_current++;
                }
            } else if (m_value_used[entry_current] == BUCKET_DELETED)
                entry_current++;
            else {
                return false;
            }

            if (entry_current == m_size) {
                entry_current = 0;
            }
        } while (entry_current != entry_start);

        return false;
    }

    void dump() {
        std::cout << "Hash map size = " << m_size
                  << ", log size = " << m_log_size << std::endl;
        std::cout << "Hash map, used = " << m_used
                  << ", used and deleted = " << m_used_and_deleted << std::endl;
        std::cout << "Hash map rehashing threshhold = "
                  << m_rehashing_threshhold << std::endl;

        for (int i = 0; i < m_size; i++) {
            std::cout << "Entry " << i << ": ";
            if (m_value_used[i] == BUCKET_FREE) {
                std::cout << "FREE";
            } else if (m_value_used[i] == BUCKET_DELETED) {
                std::cout << "DELETED";
            } else {
                T* v = get(m_values, i);
                size_t original_entry = m_limiter.limit_input(m_hasher(*v));
                std::cout << "USED, original entry " << original_entry
                          << ", value = " << *v;
            }
            std::cout << std::endl;
        }
    }

   private:
    static constexpr char BUCKET_FREE = 0;
    static constexpr char BUCKET_USED = 1;
    static constexpr char BUCKET_DELETED = 2;
    static constexpr size_t DEFAULT_HASH_SIZE = 16;

    char* m_values;
    char* m_value_used;

    size_t m_log_size;
    size_t m_size;
    size_t m_used;
    size_t m_used_and_deleted;
    size_t m_rehashing_threshhold;

    std::hash<T> m_hasher;

    F m_limiter;

    T* get(char* arr, size_t entry) {
        return reinterpret_cast<T*>(&arr[entry * sizeof(T)]);
    }

    static constexpr uint64_t next_pow2_log(uint64_t capacity) {
        return capacity <= 16 ? 4 : (64 - __builtin_clzl(capacity - 1));
    }

    size_t get_free_entry(char* value_used_array, int len, T* value) {
        size_t entry = m_limiter.limit_input(m_hasher(*value), len);

        // We guarantee that there will always be a place for a new item,
        // because we regrow it before it is full
        while (true) {
            if (value_used_array[entry] == BUCKET_FREE) {
                return entry;
            } else {
                entry++;
                if (entry == len) {
                    entry = 0;
                }
            }
        }

        return std::numeric_limits<size_t>::max();
    }

    void grow_and_rehash(size_t log_new_size) {
        assert(log_new_size >= m_log_size);
        size_t new_size = 1 << log_new_size;

        size_t count = 0;
        char* new_values = nullptr;
        int result =
            posix_memalign((void**)&new_values, 64, sizeof(T) * new_size);
        if (result != 0) {
            throw std::bad_alloc();
        }
        char* new_values_used = (char*)malloc(new_size);

        std::memset(new_values, 0, new_size * sizeof(T));
        std::memset(new_values_used, 0, new_size);

        for (int i = 0; i < m_size; i++) {
            if (m_value_used[i] == BUCKET_USED) {
                size_t entry =
                    get_free_entry(new_values_used, new_size, get(m_values, i));

                new_values_used[entry] = BUCKET_USED;
                std::memcpy(&new_values[entry * sizeof(T)],
                            &m_values[i * sizeof(T)], sizeof(T));
                count++;
            }
        }

        free(m_values);
        free(m_value_used);

        m_values = new_values;
        m_value_used = new_values_used;
        m_size = new_size;
        m_log_size = log_new_size;
        m_used = count;
        m_used_and_deleted = count;
        m_rehashing_threshhold = m_size * 0.7;
        m_limiter.set_limit(m_size);
    }

    enum find_single_result_e {
        NOT_FOUND = false,
        FOUND = true,
        INIT,
        RUNNING,
    };

    struct find_single_t {
        size_t entry_start;
        size_t entry_current;
        bool prefetched;
        find_single_result_e find_result;

        void init(size_t hash) {
            entry_start = hash;
            entry_current = hash;
            prefetched = false;
            find_result = find_single_result_e::INIT;
        }
    };

    find_single_result_e __attribute__((always_inline))
    find_single(const T& val, find_single_t& desc) {
        size_t entry_start = desc.entry_start;
        size_t entry_current = desc.entry_current;
        bool prefetched = desc.prefetched;

        do {
            if (m_value_used[entry_current] == BUCKET_USED) {
                T* p = get(m_values, entry_current);
                if (!prefetched) {
                    desc.prefetched = true;
                    desc.entry_current = entry_current;
                    __builtin_prefetch(p);
                    //__builtin_prefetch(p + 1);
                    //__builtin_prefetch(p + 2);
                    return find_single_result_e::RUNNING;
                }
                if (*p == val) {
                    return find_single_result_e::FOUND;
                } else {
                    entry_current++;
                }
            } else if (m_value_used[entry_current] == BUCKET_DELETED) {
                entry_current++;
            } else {
                return find_single_result_e::NOT_FOUND;
            }

            if (entry_current == m_size) {
                entry_current = 0;
            }
        } while (entry_current != entry_start);

        return find_single_result_e::NOT_FOUND;
    }
};

}  // namespace jsl