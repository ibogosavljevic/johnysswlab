#include <memory>
#include <cstring>

namespace jsl {

class bit_field {
public:
    using itype = uint64_t;

    bit_field(size_t size) :
        m_size_bits(size),
        m_size(calculate_size(size)),
        m_capacity(calculate_capacity(size)),
        m_value(new itype[m_capacity]) { }

    bit_field(const bit_field& other) :
        m_size_bits(other.m_size_bits),
        m_size(other.m_size),
        m_capacity(other.m_capacity),
        m_value(new itype[m_capacity]){ 
            std::memcpy(m_value.get(), other.m_value.get(), m_capacity * sizeof(itype));
    }

    bit_field(bit_field&& other) :
        m_size_bits(other.m_size_bits),
        m_size(other.m_size),
        m_capacity(other.m_capacity),
        m_value(std::move(other.m_value)) { }

    void operator=(const bit_field& other) { 
        m_size_bits = other.m_size_bits;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_value = std::unique_ptr<itype[]>(new itype[m_capacity]);

        std::memcpy(m_value.get(), other.m_value.get(), m_capacity * sizeof(itype));
    }


    void operator=(bit_field&& other) { 
        m_size_bits = other.m_size_bits;
        m_size = other.m_size;
        m_capacity = other.m_capacity;
        m_value = std::move(other.m_value);
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

    void append(const bit_field& other) {

    }

    bit_field append_t(const bit_field& other) {
        bit_field result(64);

        return result;
    }



private:    
    std::unique_ptr<itype[]> m_value;
    size_t m_capacity;
    size_t m_size_bits;
    size_t m_size;

    static constexpr size_t calculate_size(size_t size_in_bits) {
        size_t size_in_bytes = size_in_bits / 8;
        size_t size_in_units = size_in_bytes / (8 * sizeof(itype));

        if ((size_in_units * sizeof(itype) * 8) < size_in_bits) {
            size_in_units++;
        }

        return size_in_units;
    }

    static constexpr size_t calculate_capacity(size_t size_in_bits) {
        size_t size_in_units = calculate_size(size_in_bits);

        if (size_in_units < 8) {
            size_in_units = 8;
        }

        return size_in_units;
    }

    void resize(size_t new_size_bits) { 
        size_t new_capacity = calculate_capacity(new_size_bits);
        std::unique_ptr<itype[]> new_value = std::unique_ptr<itype[]>(new itype[new_capacity]);

    }
};

}