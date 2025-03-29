#pragma once

#if defined(_WIN64)
#define JSL_USE_WINDOWS
#elif defined(__unix__)
#define JSL_USE_POSIX
#define JSL_USE_JEMALLOC
#else
#error Unknown architecture
#endif

#include <cstdlib>
#include <stdexcept>
#include <initializer_list>
#include <iostream>
#if defined(JSL_USE_WINDOWS)
#include <windows.h>
#endif
#if defined(JSL_USE_POSIX)
#include <sys/mman.h>
#include <unistd.h>
#if defined(JSL_USE_JEMALLOC)
#include <jemalloc/jemalloc.h>
#endif
#endif

#include <random> 
#include <cassert>

#ifdef DEBUG_OUT
#define DBG_OUT std::cout
#else
#define DBG_OUT null_stream()
#endif

namespace jsl {

    class null_stream {
        public:
        null_stream() { }
        template<typename T> null_stream& operator<<(T const&) { return *this; }
    };

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

#ifdef JSL_USE_POSIX
    class resize_allocator_posix {
    public:
        static void* alloc(std::size_t size) {
            DBG_OUT << "Allocate size " << size << "\n";

            std::random_device rd;
            std::mt19937_64 gen(rd()); // 64-bit Mersenne Twister
            std::uniform_int_distribution<std::size_t> dist(0, static_cast<std::size_t>(4) * 1024 * 1024 * 1024);
        
            void * random_addr;
            uintptr_t addr = ((uintptr_t) &random_addr) & 0xFFFFFFFF00000000ULL + dist(gen) << 4;
            void * res = mmap((void*) addr, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

            if (res == MAP_FAILED) {
                return nullptr;
            } else {
                return res;
            }
        }

        static bool resize(void* ptr, std::size_t old_size, std::size_t new_size) {
            void * res = mremap(ptr, old_size, new_size, 0);
            DBG_OUT << "Resize, old size " << old_size << ", new size " << new_size << ", resize successfull " << (res != MAP_FAILED) << "\n" ;
            if (res == MAP_FAILED) {
                DBG_OUT << "Errno " << errno << "\n";
            }
            return res != MAP_FAILED;
        }

        static void free(void* ptr, std::size_t size) {
            DBG_OUT << "Free size " << size << "\n";
            munmap(ptr, size);
        }
    };

#endif

#if defined(JSL_USE_JEMALLOC)

class resize_allocator_jemalloc {
    public:
        static void* alloc(std::size_t size) {
            return ::malloc(size);
        }

        static bool resize(void* ptr, std::size_t old_size, std::size_t new_size) {
            size_t allocated_size = ::xallocx(ptr, new_size, 0, 0);
            if (allocated_size >= new_size) {
                DBG_OUT << "Resize from " << old_size << " to " << new_size << " succedded\n";
                return true;
            } else {
                DBG_OUT << "Resize from " << old_size << " to " << new_size << " failed\n";
                return false;
            }
        }

        static void free(void* ptr, std::size_t size) {
            ::free(ptr);
        }
    };

#endif

class resize_allocator {
    public:
        static void* alloc(std::size_t size) {
            DBG_OUT << "Allocate size " << size << "\n";

            void* a = nullptr;

            size_t page_count = to_page_count(size + sizeof(alloc_metadatata_t));
            
            for (size_t i = 0; i < TRY_COUNT; i++) {
                void* start_addr = get_random_address();
                a = allocate_pages(start_addr, page_count);
                if (a) {
                    break;
                }
            }

            if (!a) {
                return nullptr;
            }

            DBG_OUT << "Allocated " << size << "\n";

            alloc_metadatata_t* metadata = reinterpret_cast<alloc_metadatata_t*>(a);
            new (metadata) alloc_metadatata_t();
            
            metadata->total_bytes = sizeof(alloc_metadatata_t) + size;
            metadata->total_pages = page_count;
            metadata->this_ptr_page_cnt = page_count;

            return (void*) ((char*) a + sizeof(alloc_metadatata_t));
            
        }

        static bool resize(void* ptr, std::size_t old_size, std::size_t new_size) {
            DBG_OUT << "Resize, old size " << old_size << ", new size " << new_size <<  "\n" ;
            
            if (new_size <= old_size) {
                return true;
            }

            alloc_metadatata_t* metadata = reinterpret_cast<alloc_metadatata_t*>((char*) ptr - sizeof(alloc_metadatata_t));
            assert(metadata->total_bytes == (old_size + sizeof(alloc_metadatata_t)));
            assert(metadata->test_pattern == alloc_metadatata_t::TEST_PATTERN);

            size_t available_bytes = metadata->total_pages * get_page_size() - metadata->total_bytes;
            size_t needed_bytes = new_size - old_size;

            if (available_bytes >= needed_bytes) {
                metadata->total_bytes = sizeof(alloc_metadatata_t) + new_size;
                DBG_OUT << "Realloc succ, no allocation needed\n";
                return true;
            }

            // We cannot allocate more data
            if (metadata->block_cnt >= alloc_metadatata_t::MAX_ALLOC_COUNT) {
                DBG_OUT << "Realloc fail, too many pages\n";
                return false;
            }

            // Need to do page allocation
            // Figure out how many pages we need
            size_t needed_pages = to_page_count(needed_bytes - available_bytes);
            void* needed_addr = (void*) ((char*) metadata + metadata->total_pages * get_page_size());
            void* allocated_ptr = allocate_pages(needed_addr, needed_pages);

            if (allocated_ptr != needed_addr) {
                //assert(allocated_ptr == nullptr);
                DBG_OUT << "Realloc fail, no address available: ";
                DBG_OUT << "requested addr " << needed_addr << ", allocated addr " << allocated_ptr << "\n";
                return false;
            }

            const size_t idx = metadata->block_cnt;
            metadata->block_addr[idx] = allocated_ptr;
            metadata->block_page_cnt[idx] = needed_pages;
            metadata->block_cnt++;
            metadata->total_bytes = sizeof(alloc_metadatata_t) + new_size;
            metadata->total_pages += needed_pages;

            DBG_OUT << "Reallocation succeded\n";
            return true;
        }

        static void free(void* ptr, std::size_t size) {
            alloc_metadatata_t* metadata = reinterpret_cast<alloc_metadatata_t*>((char*) ptr - sizeof(alloc_metadatata_t));
            assert(metadata->test_pattern == alloc_metadatata_t::TEST_PATTERN);

            for (size_t i = 0; i < metadata->block_cnt; i++) {
                free_pages(metadata->block_addr[i], metadata->block_page_cnt[i]);
            }

            free_pages(metadata, metadata->this_ptr_page_cnt);

            DBG_OUT << "Free size " << size << "\n";
        }

    private:
        static constexpr size_t TRY_COUNT = 10;

        static void* get_random_address() {
            static std::random_device rd;
            static std::mt19937_64 gen(rd()); // 64-bit Mersenne Twister
            static std::uniform_int_distribution<std::size_t> dist(0, static_cast<std::size_t>(4) * 1024 * 1024 * 1024);
    
            static uintptr_t random_base = 0;
            if (random_base == 0) {
                random_base = (uintptr_t) allocate_pages(nullptr, 1);
                free_pages((void*) random_base, 1);
            }

            uintptr_t addr = ((((uintptr_t) random_base) & 0xFFFFFFFF00000000ULL) + dist(gen) << 4) & (~(get_page_size() - 1ULL));
            return (void*) addr;
        }

        struct alignas(32) alloc_metadatata_t {
            static constexpr size_t MAX_ALLOC_COUNT = 20ULL;

            void* block_addr[MAX_ALLOC_COUNT];
            size_t block_page_cnt[MAX_ALLOC_COUNT];
            size_t block_cnt;
            size_t test_pattern;
            size_t total_bytes;
            size_t total_pages;
            size_t this_ptr_page_cnt;

            alloc_metadatata_t() {
                block_cnt = 0;
                test_pattern = TEST_PATTERN;
            }

            static constexpr size_t TEST_PATTERN = 0x1234432156788765ULL;
        };

        static size_t get_page_size() {
            static size_t page_size = 0;

            if (page_size == 0) {
#if defined(JSL_USE_WINDOWS)
                SYSTEM_INFO si;
                GetSystemInfo(&si);
                page_size = std::max(si.dwPageSize, si.dwAllocationGranularity);
#elif defined(JSL_USE_POSIX)
                page_size = sysconf(_SC_PAGE_SIZE);
#endif
            }
            return page_size;
        }

        static size_t to_page_count(const size_t bytes) {
            const size_t page_size = get_page_size();
            return (page_size + bytes - 1) / page_size;
        }

        static void* allocate_pages(void* address, size_t page_count) {
#if defined(JSL_USE_WINDOWS)
            void* r = (void*) VirtualAlloc(address, page_count * get_page_size(), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(JSL_USE_POSIX)
            void* r = mmap((void*) address, page_count * get_page_size(), PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
            if (r == MAP_FAILED) {
                r = nullptr;
            }
#endif
            return r;
        }

        static void free_pages(void* address, size_t page_count) {
#if defined(JSL_USE_WINDOWS)
            VirtualFree(address, 0, MEM_RELEASE);
#elif defined(JSL_USE_POSIX)
            munmap(address, page_count * get_page_size());
#endif
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
