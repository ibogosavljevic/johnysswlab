#include <utility>
#include <string>
#include <sys/mman.h>

namespace jsl {

template <unsigned int SIZE, typename KeyType, typename ValueType>
class oa_hash_map {
private:
    static constexpr KeyType EXTRACT_CONTROL_BITS = 3;

    static constexpr KeyType ENTRY_FREE = 0;
    static constexpr KeyType ENTRY_USED = 1;
    static constexpr KeyType ENTRY_DELETED = 2;

    static constexpr KeyType ADD_CONTROL_BITS = 2;

    KeyType next_entry(KeyType entry) {
        return (entry + 1 ) % SIZE;
    }

    template <bool force_emplace, typename... Args>
    std::pair<bool, ValueType*> emplace_private(KeyType key, const Args &... args) {
        KeyType bucket = key % SIZE;
        KeyType key_with_ctrl_bits = (key << ADD_CONTROL_BITS) | ENTRY_USED;
        bool free_bucket_found = false;
        KeyType free_bucket_index;

        while(true) {
            KeyType control_bits = m_hasharray[bucket].key & EXTRACT_CONTROL_BITS;
            switch(control_bits) {
                case ENTRY_FREE: {
                    // End of chain, a free bucket has been found
                    goto exit_while;
                }
                case ENTRY_DELETED: {
                    if (!free_bucket_found) {
                        free_bucket_found = true;
                        free_bucket_index = bucket;
                    }
                    bucket = next_entry(bucket);
                    break;
                }
                case ENTRY_USED: {
                    if (m_hasharray[bucket].key == key_with_ctrl_bits) {
                        if (force_emplace) {
                            m_hasharray[bucket].get_value()->~ValueType();
                            free_bucket_found = true;
                            free_bucket_index = bucket;
                            goto exit_while;
                        } else {
                            return { false, m_hasharray[bucket].get_value() };
                        }
                    } else {
                        bucket = next_entry(bucket);
                    }
                    break;
                }
            }
        }

exit_while:
        if (free_bucket_found) {
            bucket = free_bucket_index;
        }

        m_hasharray[bucket].key = key_with_ctrl_bits;
        new (m_hasharray[bucket].value) ValueType(args...);
        return { true, m_hasharray[bucket].get_value() };
    }

public:
    oa_hash_map(bool use_large_pages) {
        int large_pages = use_large_pages ? MAP_HUGETLB : 0;
        m_hasharray = reinterpret_cast<KeyValueType*>(mmap(0, SIZE * sizeof(KeyValueType), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|large_pages, -1, 0));
        new (m_hasharray) KeyValueType[SIZE];
    }

    ~oa_hash_map() {
        for (int i = 0; i < SIZE; i++) {
            m_hasharray[i].~KeyValueType();
        }
        munmap(m_hasharray, SIZE * sizeof(KeyValueType));
    }

    ValueType* get(const KeyType& key) {
        KeyType bucket = key % SIZE;
        KeyType key_with_ctrl_bits = (key << ADD_CONTROL_BITS) | ENTRY_USED;

        while (true) {
            KeyType control_bits = m_hasharray[bucket].key & EXTRACT_CONTROL_BITS;
            switch (control_bits) {
                case ENTRY_USED: {
                    if (key_with_ctrl_bits == m_hasharray[bucket].key) {
                        return m_hasharray[bucket].get_value(); 
                    } else {
                        bucket = next_entry(bucket);
                    }
                    break;
                }
                case ENTRY_DELETED: {
                    bucket = next_entry(bucket);
                    break;
                }
                case ENTRY_FREE: {
                    return nullptr;
                }
            }
        }

        return nullptr;
    }


    template <typename... Args>
    std::pair<bool, ValueType*> try_emplace(KeyType key, const Args &... args) {
        return emplace_private<false>(key, args...);
    }

    template <typename... Args>
    ValueType* emplace(KeyType key, const Args &... args) {
        return emplace_private<true>(key, args...).second;
    }

    bool remove(KeyType key) {
        KeyType bucket = key % SIZE;
        KeyType key_with_ctrl_bits = (key << ADD_CONTROL_BITS) | ENTRY_USED;

        while(true) {
            KeyType control_bits = m_hasharray[bucket].key & EXTRACT_CONTROL_BITS;
            switch (control_bits) {
                case ENTRY_USED: {
                    if (m_hasharray[bucket].key == key_with_ctrl_bits) {
                        m_hasharray[bucket].get_value()->~ValueType();

                        KeyType next_bucket = next_entry(bucket);
                        if ((m_hasharray[next_bucket].key & EXTRACT_CONTROL_BITS) == ENTRY_FREE) {
                            m_hasharray[bucket].key = ENTRY_FREE;
                        } else {
                            m_hasharray[bucket].key = ENTRY_DELETED;
                        }

                        return true;
                    } else {
                        bucket = next_entry(bucket);
                    }

                    break;
                }
                case ENTRY_DELETED: {
                    bucket = next_entry(bucket);
                    break;
                }
                case ENTRY_FREE : {
                    return false;
                }
            }
        }
    }

    std::string get_statistics() {
        size_t free_buckets = 0;
        size_t deleted_buckets = 0;
        size_t misplaced_buckets = 0;
        size_t correctly_placed_buckets = 0;

        for (size_t i = 0; i < SIZE; i++) {
            KeyType control_bits = m_hasharray[i].key & EXTRACT_CONTROL_BITS;
            switch (control_bits) {
                case ENTRY_FREE:
                    free_buckets++;
                    break;
                case ENTRY_DELETED:
                    deleted_buckets++;
                    break;
                case ENTRY_USED: {
                    KeyType bucket = (m_hasharray[i].key >> ADD_CONTROL_BITS) % SIZE;
                    if (bucket == i) {
                        correctly_placed_buckets++;
                    } else {
                        misplaced_buckets++;
                    }
                    break;
                }
            }
        }

        std::string result;
        result.reserve(300);
        result += "Total buckets: " + std::to_string(SIZE) + "\n";
        result += "Used buckets: " + std::to_string(misplaced_buckets + correctly_placed_buckets) + "\n";
        result += "Correctly placed buckets: " + std::to_string(correctly_placed_buckets) + "\n";
        result += "Misplaced buckets: " + std::to_string(misplaced_buckets) + "\n";
        result += "Deleted buckets: " + std::to_string(deleted_buckets) + "\n";

        return result;
    }

private:
    static constexpr bool is_power_of_two(unsigned int value) {
        return (value & (value - 1)) == 0;
    }

    static_assert(is_power_of_two(SIZE), "Hash map size must be a power of two");

    struct KeyValueType {
        KeyType key;
        alignas(alignof(ValueType)) char value[sizeof(ValueType)];

        ValueType* get_value() {
            return reinterpret_cast<ValueType*>(value);
        }

        KeyValueType() : key(ENTRY_FREE) {}
        ~KeyValueType() {
            KeyType control_bits = key & EXTRACT_CONTROL_BITS;
            if (key == ENTRY_USED) {
                get_value()->~ValueType();
            }
        }
    };

    KeyValueType* m_hasharray;
};

}