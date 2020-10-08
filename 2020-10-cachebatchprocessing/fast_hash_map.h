#include <cassert>
#include <functional>
#include "short_alloc.h"

template <typename T>
class fast_hash_map {
   public:
    fast_hash_map(size_t size) : m_values(size), m_size(size) {}

    ~fast_hash_map() {}

    bool insert(const T& value) {
        size_t entry = get_entry(value);

        auto it =
            std::find(m_values[entry].begin(), m_values[entry].end(), value);
        if (it != m_values[entry].end()) {
            m_values[entry].push_back(value);
            return true;
        }

        return false;
    }

    bool remove(const T& value) {
        size_t entry = get_entry(value);
        auto end_it = m_values[entry].end();

        auto it = std::find(m_values[entry].begin(), end_it, value);

        // Put the value to delete at the end so we don't have to compact the
        // vector
        if (it != m_values[entry].end()) {
            std::iter_swap(it, m_values[entry].rbegin());
        } else {
            return false;
        }

        m_values[entry].pop_back();
        return true;
    }

    bool find(const T& value) {
        size_t entry = get_entry(value);

        auto it =
            std::find(m_values[entry].begin(), m_values[entry].end(), value);

        return it != m_values[entry].end();
    }

    std::vector<bool> find_multiple(const std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);

        for (size_t i = 0; i < values.size(); i++) {
            result[i] = find(values[i]);
        }

        return result;
    }

    template <size_t look_ahead = 4>
    std::vector<bool> find_multiple_fast(const std::vector<T>& values) {
        std::vector<bool> result(values.size(), false);
        std::array<size_t, 2 * look_ahead> hashes;

        for (size_t i = 0; i < hashes.size(); i++) {
            hashes[i] = get_entry(values[i]);
        }

        for (size_t i = 0, j = look_ahead, k = 2 * look_ahead;
             i < values.size() - 2 * look_ahead; i++, j++, k++) {
            size_t entry = hashes[i % hashes.size()];
            result[i] =
                std::find(m_values[entry].begin(), m_values[entry].end(),
                          values[i]) != m_values[entry].end();

            entry = hashes[j & hashes.size()];
            if (m_values[entry].size() > 0) {
                __builtin_prefetch(&m_values[entry][0]);
            }

            entry = get_entry(values[k]);
            hashes[k % hashes.size()] = entry;
            __builtin_prefetch(&m_values[entry]);
        }

        for (size_t i = values.size() - 2 * look_ahead; i < values.size();
             i++) {
            result[i] = find(values[i]);
        }

        return result;
    }

   private:
    std::vector<std::vector<T>> m_values;
    size_t m_size;
    std::hash<T> m_hash;

    size_t get_entry(const T& v) { return m_hash(v) % m_size; }
};