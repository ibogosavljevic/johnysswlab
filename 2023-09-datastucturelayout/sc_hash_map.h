#include <vector>
#include <utility>
#include <cstdint>

namespace jsl {

template <unsigned int SIZE, typename KeyType, typename ValueType, bool debug = false>
class sc_hash_map {
public:
    sc_hash_map() {
        m_hasharray = new KeyValueType[SIZE];
        m_free_list = END_OF_LIST;
    }

    ~sc_hash_map() {
        delete [] m_hasharray;
    }

    ValueType* get(KeyType key) {
        KeyType bucket = key % SIZE;

        if (m_hasharray[bucket].next == EMPTY_BUCKET) {
            return nullptr;
        }

        if (m_hasharray[bucket].key == key) {
            return m_hasharray[bucket].get_value();
        }

        // Moving to conflict list
        uint32_t current = m_hasharray[bucket].next;

        while (current != END_OF_LIST) {
            if (key == m_collision_array[current].key) {
                return m_collision_array[current].get_value();
            }
            current = m_collision_array[current].next;
        }

        return nullptr;
    }

    bool remove(KeyType key) {
        KeyType bucket = key % SIZE;

        if (m_hasharray[bucket].next == EMPTY_BUCKET) {
            return false;
        }

        if (m_hasharray[bucket].key == key) {
            m_hasharray[bucket].get_value()->~ValueType();

            if (m_hasharray[bucket].next == END_OF_LIST) {
                m_hasharray[bucket].next = EMPTY_BUCKET;
                return true;
            } else {
                uint32_t next = m_hasharray[bucket].next;
                new (m_hasharray[bucket].value) ValueType(*m_collision_array[next].get_value());
                m_hasharray[bucket].next = m_collision_array[next].next;
                m_hasharray[bucket].key = m_collision_array[next].key;
                m_collision_array[next].get_value()->~ValueType();

                m_collision_array[next].next = m_free_list;
                m_free_list = next;
                return true;
            }
        }

        // Moving to conflict list
        uint32_t current = m_hasharray[bucket].next;
        KeyValueType* prev = &m_hasharray[bucket];

        while (current != END_OF_LIST) {
            if (key == m_collision_array[current].key) {
                m_collision_array[current].get_value()->~ValueType();
                prev->next = m_collision_array[current].next;
                
                m_collision_array[current].next = m_free_list;
                m_free_list = current;
                return true;
            } else {
                prev = &m_collision_array[current];
                current = m_collision_array[current].next;
            }
        }

        return false;
    }


    template <typename... Args>
    std::pair<bool, ValueType*> try_emplace(KeyType key, const Args&... args) {
        return emplace_private<false>(key, args...);
    }

    template <typename... Args>
    ValueType* emplace(KeyType key, const Args&... args) {
        return emplace_private<true>(key, args...).second;
    }

    std::string get_statistics() {
        size_t free_buckets = 0;
        size_t used_buckets = 0;
        size_t collisions = 0;

        for (size_t i = 0; i < SIZE; i++) {
            if (m_hasharray[i].next == EMPTY_BUCKET) {
                free_buckets++;
            } else {
                used_buckets++;
                
                uint32_t current = m_hasharray[i].next;
                while(current != END_OF_LIST) {
                    collisions++;
                    current = m_collision_array[current].next;
                }
            }
        }

        std::string result;
        result.reserve(300);
        result += "Total buckets: " + std::to_string(SIZE) + "\n";
        result += "Total values: " + std::to_string(used_buckets + collisions) + "\n";
        result += "Used buckets: " + std::to_string(used_buckets) + "\n";
        result += "Collisions: " + std::to_string(collisions) + "\n";

        return result;
    }

private:
    static constexpr uint32_t EMPTY_BUCKET = static_cast<uint32_t>(-2);
    static constexpr uint32_t END_OF_LIST = static_cast<uint32_t>(-1);

    struct KeyValueType {
        KeyType key;
        alignas(alignof(ValueType)) char value[sizeof(ValueType)];

        uint32_t next;

        ValueType* get_value() {
            return reinterpret_cast<ValueType*>(value);
        }

        KeyValueType() : next(EMPTY_BUCKET) {}
        ~KeyValueType() {
            if (next != EMPTY_BUCKET) {
                get_value()->~ValueType();
            }
        }
    };

    template <bool force_emplace, typename... Args>
    std::pair<bool, ValueType*> emplace_private(KeyType key, const Args&... args) {
        KeyType bucket = key % SIZE;

        if (m_hasharray[bucket].next == EMPTY_BUCKET) {
            m_hasharray[bucket].key = key;
            m_hasharray[bucket].next = END_OF_LIST;
            new (m_hasharray[bucket].value) ValueType(args...);
            return { true, m_hasharray[bucket].get_value() };
        }

        if (m_hasharray[bucket].key == key) {
            if (force_emplace) {
                m_hasharray[bucket].get_value()->~ValueType();
                new (m_hasharray[bucket].value) ValueType(args...);
                return { true, m_hasharray[bucket].get_value() };
            } else {
                return { false, m_hasharray[bucket].get_value() };
            }
        }

        // Conflict list
        uint32_t current = m_hasharray[bucket].next;
        while (current != END_OF_LIST) {
            if (m_collision_array[current].key == key) {
                if (force_emplace) {
                    m_collision_array[current].get_value()->~ValueType();
                    new (m_collision_array[current].value) ValueType(args...);
                    return { true, m_collision_array[current].get_value() };
                } else {
                    return { false, m_collision_array[current].get_value() };
                }
            }
            current = m_collision_array[current].next;
        }

        size_t index;
        // Nothing found, but we have a conflict
        // We need to allocate a new 
        if (m_free_list == END_OF_LIST) {
            m_collision_array.emplace_back();
            index = m_collision_array.size() - 1;
        } else {
            index = m_free_list;
            m_free_list = m_collision_array[m_free_list].next;
        }

        m_collision_array[index].key = key;
        new (m_collision_array[index].value) ValueType(args...);
        m_collision_array[index].next = m_hasharray[bucket].next;
        m_hasharray[bucket].next = index;
        return { true, m_collision_array[index].get_value() };
    }

    KeyValueType* m_hasharray;
    std::vector<KeyValueType> m_collision_array;
    uint32_t m_free_list;
};

}