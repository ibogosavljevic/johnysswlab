#include <cstdlib>
#include <stdexcept>
#include <initializer_list>
#include <iostream>
#include <sys/mman.h>

namespace jsl {

    class simple_allocator {
    public:
        static void* alloc(std::size_t size) {
            return std::malloc(size);
        }

        static bool resize(void* ptr, std::size_t old_size, std::size_t new_size) {
            return false;
        }

        static void free(void* ptr, std::size_t size) {
            std::free(ptr);
        }
    };

    class resize_allocator {
    public:
        static void* alloc(std::size_t size) {
            std::cout << "Allocate size " << size << "\n";
            void * res = mmap((void*) 0x7f02ca14c000, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

            if (res == MAP_FAILED) {
                return nullptr;
            } else {
                return res;
            }
        }

        static bool resize(void* ptr, std::size_t old_size, std::size_t new_size) {
            void * res = mremap(ptr, old_size, new_size, 0);
            std::cout << "Resize, old size " << old_size << ", new size " << new_size << ", resize successfull " << (res != MAP_FAILED) << "\n" ;
            if (res == MAP_FAILED) {
                std::cout << "Errno " << errno << "\n";
            }
            return res != MAP_FAILED;
        }

        static void free(void* ptr, std::size_t size) {
            std::cout << "Free size " << size << "\n";
            munmap(ptr, size);
        }
    };

    template <typename T, typename Allocator>
    class vector {
    private:
        T* data;
        std::size_t sz;
        std::size_t cap;

        void resize_capacity(std::size_t new_cap) {
            if (!data) {
                data = reinterpret_cast<T*>(Allocator::alloc(new_cap * sizeof(T)));
                if (!data) {
                    throw std::bad_alloc();
                }
            } else {
                bool resize_succ = Allocator::resize(data, cap * sizeof(T), new_cap * sizeof(T));

                if (!resize_succ) {
                    T* new_data = reinterpret_cast<T*>(Allocator::alloc(new_cap * sizeof(T)));
                    if (!new_data) {
                        throw std::bad_alloc();
                    }

                    for (std::size_t i = 0; i < sz; ++i) {
                        new (&new_data[i]) T(std::move(data[i]));
                        data[i].~T();
                    }

                    Allocator::free(data, cap);
                    data = new_data;
                }
            }

            cap = new_cap;
        }
    
    public:
        vector() : data(nullptr), sz(0), cap(0) {}

        vector(std::initializer_list<T> init) : vector() {
            reserve(init.size());
            for (const auto& val : init) push_back(val);
        }

        ~vector() {
            clear();
            if (data) {
                Allocator::free(data, cap);
            }
        }

        void push_back(const T& value) {
            if (sz == cap) resize_capacity(cap ? cap * 2 : 1);
            new (&data[sz++]) T(value);
        }

        void pop_back() {
            if (sz == 0) throw std::out_of_range("pop_back() on empty vector");
            data[--sz].~T();
        }

        T& operator[](std::size_t index) {
            return data[index];
        }

        const T& operator[](std::size_t index) const {
            return data[index];
        }

        std::size_t size() const { return sz; }

        void clear() {
            for (std::size_t i = 0; i < sz; ++i) data[i].~T();
            sz = 0;
        }

        void reserve(std::size_t new_cap) {
            if (new_cap > cap) resize_capacity(new_cap);
        }
    };
}
