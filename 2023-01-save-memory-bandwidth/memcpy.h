#include <immintrin.h>
#include <cstdint>

// Always returs 0, but to the CPU it
// seems like there is a real dependency
int always_zero_with_dependency(int val) {
    int val2;
    int res = val;
    __asm__ (
        "mov %[val], %[val2];"
        "sub %[val2], %[res]"
        : [val] "+&r"(val), [val2] "+&r" (val2), [res] "+&r" (res)
        :
        : "cc"
    );
    return res;
}


void memcpy_simple(uint32_t* dst, uint32_t* src, int n) {
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (src + i);
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_storeu_si256(store_addr, val);
    }
}

void memcpy_fake_dep(uint32_t* dst, uint32_t* src, int n) {
    uint32_t* addr = src;
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (addr);
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_storeu_si256(store_addr, val);

         addr += always_zero_with_dependency( _mm256_cvtsi256_si32 (val)) + 8;
    }
}

void memcpy_fence(uint32_t* dst, uint32_t* src, int n) {
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (src + i);
        _mm_lfence();
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_storeu_si256(store_addr, val);
    }
}

void memcpy_streaming_stores(uint32_t* dst, uint32_t* src, int n) {
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (src + i);
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_stream_si256(store_addr, val);
    }
}

void memcpy_nt_loads(uint32_t* dst, uint32_t* src, int n) {
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (src + i);
        _mm_prefetch(load_addr, _MM_HINT_NTA);
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_storeu_si256(store_addr, val);
    }
}

void memcpy_streaming_stores_nt_loads(uint32_t* dst, uint32_t* src, int n) {
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (src + i);
        _mm_prefetch(load_addr, _MM_HINT_NTA);
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_stream_si256(store_addr, val);
    }
}

void memcpy_streaming_stores_nt_loads_fence(uint32_t* dst, uint32_t* src, int n) {
    for (int i = 0; i < n; i+= 8) {
        __m256i* load_addr = (__m256i*) (src + i);
        _mm_prefetch(load_addr, _MM_HINT_NTA);
        _mm_lfence();
        __m256i val = _mm256_lddqu_si256(load_addr);
        __m256i* store_addr = (__m256i*) (dst + i);
        _mm256_stream_si256(store_addr, val);
    }
}