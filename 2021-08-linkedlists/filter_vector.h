#pragma once

#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace jsl {

template <typename T>
class filter_vector {
   public:
    using iterator = boost::dynamic_bitset<uint32_t>::size_type;
    using size_type = boost::dynamic_bitset<uint32_t>::size_type;

    iterator begin() { return m_used.find_first(); }

    iterator end() { return boost::dynamic_bitset<uint32_t>::npos; }

    iterator next(iterator current) { return m_used.find_next(current); }

    T& at(iterator it) { return *reinterpret_cast<T*>(m_values[it].val); }

    void push_back(const T& val) {
        m_values.emplace_back();
        m_used.push_back(true);
        m_size++;

        new (m_values.back().val) T(val);
    }

    iterator erase(iterator it) {
        at(it).~T();
        m_used[it] = false;
        m_size--;
        return m_used.find_next(it);
    }

    void reserve(size_type size) {
        m_used.reserve(size);
        m_values.reserve(size);
    }

    void compact() {
        size_type current_free = end();

        for (int i = 0; i < m_used.size(); i++) {
            if (!m_used[i]) {
                current_free = i;
                break;
            }
        }

        if (current_free != end()) {
            size_type current_used = m_used.find_next(current_free);

            while (current_used != end()) {
                T& used = at(current_used);
                new (m_values[current_free].val) T(std::move(used));
                used.~T();
                m_used[current_free] = true;
                m_used[current_used] = false;

                current_used = m_used.find_next(current_used);
                current_free++;
            }
        }
    }

   private:
    struct elem_t {
        char val[sizeof(T)];
    };

    std::vector<elem_t> m_values;
    boost::dynamic_bitset<uint32_t> m_used;
    size_type m_size;
};

}  // namespace jsl