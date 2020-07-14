#include <cstdlib>
#include <stddef.h>
#include <cassert>

template <typename _Tp>
class custom_allocator
{
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _Tp* pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp& reference;
    typedef const _Tp& const_reference;
    typedef _Tp value_type;

    template <typename _Tp1>
    struct rebind {
        typedef custom_allocator<_Tp1> other;
    };

    custom_allocator()
    {
    }

    custom_allocator(const custom_allocator&)
    {
    }

    template <typename _Tp1>
    custom_allocator(const custom_allocator<_Tp1>&)
    {
    }

    ~custom_allocator()
    {
    }

    pointer address(reference __x) const
    {
        return std::__addressof(__x);
    }

    const_pointer address(const_reference __x) const
    {
        return std::__addressof(__x);
    }

    pointer allocate(size_type __n, const void* = 0)
    {
        return static_cast<_Tp*>(std::malloc(__n * sizeof(_Tp) * 2));
    }

    void deallocate(pointer __p, size_type __n)
    {
        std::free(__p);
    }

    size_type max_size() const
    {
        return size_t(-1) / sizeof(_Tp);
    }

    void construct(pointer __p, const _Tp& __val)
    {
        ::new ((void*)__p) _Tp(__val);
    }

    void destroy(pointer __p)
    {
        __p->~_Tp();
    }
};


#include <sys/mman.h>

template <typename _Tp, int gap>
class zone_allocator
{
private:
    _Tp* my_memory;
    _Tp* last_location;
    int free_block_index;
    int start_index;
    static constexpr int mem_size = 1000*1024*1024;
public:
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef _Tp* pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp& reference;
    typedef const _Tp& const_reference;
    typedef _Tp value_type;

    template <typename _Tp1>
    struct rebind {
        typedef zone_allocator<_Tp1, gap> other;
    };

    zone_allocator()
    {
        char * p = reinterpret_cast<char*>(mmap(0, mem_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0));
        my_memory = reinterpret_cast<_Tp*>(p);
        last_location = reinterpret_cast<_Tp*>(p + mem_size);
        free_block_index = 0;
        start_index = 0;
    }

    zone_allocator(const zone_allocator&)
    {
    }

    template <typename _Tp1, int gap1>
    zone_allocator(const zone_allocator<_Tp1, gap1>&)
    {
    }

    ~zone_allocator()
    {
        munmap(my_memory, mem_size);
    }

    pointer address(reference __x) const
    {
        return std::__addressof(__x);
    }

    const_pointer address(const_reference __x) const
    {
        return std::__addressof(__x);
    }

    pointer allocate(size_type __n, const void* = 0)
    {
        assert(__n == 1);
        pointer result = &my_memory[free_block_index];
        free_block_index += __n + gap;

        if (&my_memory[free_block_index] >= last_location) {
            start_index++;
            free_block_index = start_index;
        }
        return result;
    }

    void deallocate(pointer __p, size_type __n)
    {
        // We deallocate everything when destroyed
    }

    size_type max_size() const
    {
        return size_t(-1) / sizeof(_Tp);
    }

    void construct(pointer __p, const _Tp& __val)
    {
        ::new ((void*)__p) _Tp(__val);
    }

    void destroy(pointer __p)
    {
        __p->~_Tp();
    }
};