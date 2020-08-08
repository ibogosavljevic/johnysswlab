#include <cstdlib>
#include <stddef.h>
#include <cassert>

#include <sys/mman.h>

template
<typename T>
class zone_allocator_config {
public:
    static bool get_use_large_pages() { return m_use_large_pages; }
    static void set_use_large_pages(bool use_large_pages) { m_use_large_pages = use_large_pages; }
private:
    static bool m_use_large_pages;
};

template <typename T>
bool zone_allocator_config<T>::m_use_large_pages = false;

template <class T, int gap>
class zone_allocator {
private:
    T* my_memory;
    T* last_location;
    int free_block_index;
    int start_index;
    static constexpr int mem_size = 1024*1024*1024;
public:
    typedef T value_type;
    zone_allocator() noexcept {
        int large_pages = zone_allocator_config<void>::get_use_large_pages() ? MAP_HUGETLB : 0;
        char * p = reinterpret_cast<char*>(mmap(0, mem_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0));
        my_memory = reinterpret_cast<T*>(p);
        last_location = reinterpret_cast<T*>(p + mem_size);
        free_block_index = 0;
        start_index = 0;

        std::cout << "Type size = " << sizeof(T) << std::endl;
    }
    ~zone_allocator() {
        munmap(my_memory, mem_size);
    }

    template<class U>
    struct rebind { using other = zone_allocator<U, gap>; };    

    template <class U> zone_allocator (const zone_allocator<U, gap>&) noexcept {}
    T* allocate (std::size_t n) { 
        assert(n == 1);
        T* result = &my_memory[free_block_index];
        free_block_index += n + gap;

        if (&my_memory[free_block_index] >= last_location) {
            start_index++;
            free_block_index = start_index;
        }
        return result;

    }
    void deallocate (T* p, std::size_t n) { 
        // noop
     }
};

