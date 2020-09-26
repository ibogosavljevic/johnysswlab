#include <cstring>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <memory>

namespace jsl {

class bit_field {
   public:
    using itype = uint64_t;

#ifndef NO_CTOR_INIT
    bit_field(size_t size)
        : m_size_bits(size),
          m_size(calculate_size(size)),
          m_capacity(calculate_capacity(size)),
          m_value(new itype[m_capacity]) {}

    bit_field(const bit_field& other)
        : m_size_bits(other.m_size_bits),
          m_size(other.m_size),
          m_capacity(other.m_capacity),
          m_value(new itype[m_capacity]) {
        std::memcpy(m_value.get(), other.m_value.get(), m_size * sizeof(itype));
    }
#else
    bit_field(size_t size) {
        m_size_bits = size;
        m_size = calculate_size(size);
        m_capacity = calculate_capacity(size);
        m_value = std::unique_ptr<itype[]>(new itype[m_capacity]);
    }

    bit_field(const bit_field& other) {
        m_size_bits = other.m_size_bits;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_value = std::unique_ptr<itype[]>(new itype[m_capacity]);
        std::memcpy(m_value.get(), other.m_value.get(), m_size * sizeof(itype));
    }

#endif

    explicit bit_field(itype value, itype dummy)
        : m_size_bits(sizeof(itype)),
          m_size(1),
          m_capacity(1),
          m_value(new itype[m_capacity]) {
        m_value[0] = value;
    }

    explicit bit_field(std::initializer_list<itype> l)
        : m_size_bits(l.size() * 8 * sizeof(itype)),
          m_size(l.size()),
          m_capacity(m_size),
          m_value(new itype[m_capacity]) {
        int i = 0;
        for (auto it = l.begin(); it != l.end(); ++it) {
            m_value[i] = *it;
            i++;
        }
    }

#ifndef NO_MOVE_CTOR
    bit_field(bit_field&& other) noexcept
        : m_size_bits(other.m_size_bits),
          m_size(other.m_size),
          m_capacity(other.m_capacity),
          m_value(std::move(other.m_value)) {}
#endif
    void operator=(const bit_field& other) {
        if (other.m_capacity <= m_capacity) {
            std::memcpy(m_value.get(), other.m_value.get(),
                        other.m_size * sizeof(itype));

            m_size_bits = other.m_size_bits;
            m_size = other.m_size;
        } else {
            m_value = std::unique_ptr<itype[]>(new itype[other.m_capacity]);
            std::memcpy(m_value.get(), other.m_value.get(),
                        other.m_size * sizeof(itype));

            m_size_bits = other.m_size_bits;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
        }
    }

#ifndef NO_MOVE_CTR
    void operator=(bit_field&& other) noexcept {
        m_size_bits = other.m_size_bits;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_value = std::move(other.m_value);
    }
#endif

    void operator=(std::initializer_list<itype> l) {
        size_t size_in_bits = l.size() * sizeof(itype) * 8;

        if (l.size() > m_capacity) {
            resize(size_in_bits);
        }

        m_size_bits = size_in_bits;
        m_size = l.size();

        int i = 0;
        for (auto it = l.begin(); it != l.end(); ++it) {
            m_value[i] = *it;
            i++;
        }
    }

    bit_field operator&(bit_field& other) {
        bit_field result(*this);

        result &= other;
        return result;
    }

    bit_field operator|(bit_field& other) {
        bit_field result(*this);

        result |= other;
        return result;
    }

    bit_field operator^(bit_field& other) {
        bit_field result(*this);

        result ^= other;
        return result;
    }

    void operator&=(const bit_field& other) {
        size_t loop_end = std::min<size_t>(m_size, other.m_size);
        for (int i = 0; i < loop_end; ++i) {
            m_value[i] &= other.m_value[i];
        }
    }

    void operator|=(const bit_field& other) {
        size_t loop_end = std::min<size_t>(m_size, other.m_size);
        for (int i = 0; i < loop_end; ++i) {
            m_value[i] |= other.m_value[i];
        }
    }

    void operator^=(const bit_field& other) {
        size_t loop_end = std::min<size_t>(m_size, other.m_size);
        for (int i = 0; i < loop_end; ++i) {
            m_value[i] ^= other.m_value[i];
        }
    }

    bool operator==(const bit_field& other) {
        if (m_size_bits != other.m_size_bits) {
            return false;
        }

        if (m_size != other.m_size) {
            return false;
        }

        return std::memcmp(m_value.get(), other.m_value.get(),
                           m_size * sizeof(itype)) == 0;
    }

    void append(const bit_field& other) {
        size_t my_size_units;
        size_t other_size_units;

        // std::cout << "\nTHIS ";
        // stream_to(std::cout);

        // std::cout << "\nOTHER ";
        // other.stream_to(std::cout);

        if (!fits_full_unit(m_size_bits, my_size_units)) {
            throw std::logic_error("");
        }

        if (!fits_full_unit(other.m_size_bits, other_size_units)) {
            throw std::logic_error("");
        }

        if (m_capacity < (m_size + other.m_size)) {
            resize(m_size_bits + other.m_size_bits);
        }

        // std::cout << "\nTHIS AFTER RESIZE ";
        // stream_to(std::cout);

        for (size_t i = 0, j = m_size; i < other.m_size; i++, j++) {
            m_value[j] = other.m_value[i];
        }

        m_size += other.m_size;
        m_size_bits += other.m_size_bits;

        // std::cout << "\nTHIS AFTER APPEND ";
        // stream_to(std::cout);
    }

    bit_field append_t(const bit_field& other) {
        bit_field result(m_size_bits + other.m_size_bits);

        result = *this;
        result.append(other);

        return result;
    }

    void stream_to(std::ostream& os) const {
        os << "size_bits(" << m_size_bits << "), size(" << m_size
           << "), capacity (" << m_capacity << "): ";
        os << std::hex;

        for (size_t i = 0; i < m_size; i++) {
            os << m_value[i] << ", ";
        }
        os << std::dec << std::flush;
    }

    void reserve(size_t bits) { resize(bits); }

    size_t get_size() { return m_size_bits; }

   private:
    size_t m_size_bits;
    size_t m_size;
    size_t m_capacity;
    std::unique_ptr<itype[]> m_value;

    static size_t calculate_size(size_t size_in_bits) {
        size_t size_in_bytes = size_in_bits / 8;
        size_t size_in_units = size_in_bytes / sizeof(itype);

        if ((size_in_units * sizeof(itype) * 8) < size_in_bits) {
            size_in_units++;
        }

        return size_in_units;
    }

    static size_t calculate_capacity(size_t size_in_bits) {
        size_t size_in_units = calculate_size(size_in_bits);

        if (size_in_units < 8) {
            size_in_units = 8;
        }

        return size_in_units;
    }

    void resize(size_t new_size_bits) {
        size_t new_capacity = calculate_capacity(new_size_bits);
        // std::cout << "resize, old capacity" << m_capacity << "new_capacity"
        // << new_capacity; std::cout << "old size in bits" << m_size_bits << ",
        // new size in bits " << new_size_bits << std::endl;
        std::unique_ptr<itype[]> new_value =
            std::unique_ptr<itype[]>(new itype[new_capacity]);

        std::memcpy(new_value.get(), m_value.get(), m_capacity * sizeof(itype));

        m_value = std::move(new_value);
        m_capacity = new_capacity;
    }

    bool fits_full_unit(size_t size_bits, size_t& out_unit_size) {
        size_t size_bytes = size_bits / 8;
        out_unit_size = size_bytes / sizeof(itype);

        return (out_unit_size * sizeof(itype) * 8) == size_bits;
    }
};

}  // namespace jsl