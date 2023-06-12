#include <arm_neon.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include "likwid.h"

void func_simple(int *a, int* b, int* c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] * b[i];
    }
}

void func_simple_intrinsics(int *a, int* b, int* c, int n) {
    for (int i = 0; i < n; i+=4) {
        int32x4_t a_val = vld1q_s32(a + i);
        int32x4_t b_val = vld1q_s32(b + i);
        int32x4_t c_val = vmulq_s32(a_val, b_val);
        vst1q_s32(c + i, c_val);
    }
}

void func_simple_asm(int *a, int* b, int* c, int n) {
    long n_long = n;
    asm volatile (
        "mov x0, 0\n"
        "LOOP_SIMPLE%=:\n"
        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "add x0, x0, #4\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "cmp x0, %[n]\n"
        "st1 { v2.4s }, [%[c]], #16\n"
        "b.ne LOOP_SIMPLE%=\n"
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c)
        : [n] "r" (n_long)
        : "v0", "v1", "v2", "x0", "cc", "memory"
    );
}

void func_simple_unroll(int *a, int* b, int* c, int n) {
    long n_long = n;
    asm volatile (
        "mov x0, 0\n"
        "LOOP_SIMPLE%=:\n"
        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "st1 { v2.4s }, [%[c]], #16\n"

        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "st1 { v2.4s }, [%[c]], #16\n"

        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "st1 { v2.4s }, [%[c]], #16\n"

        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "add x0, x0, #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "cmp x0, %[n]\n"
        "st1 { v2.4s }, [%[c]], #16\n"

        "b.ne LOOP_SIMPLE%=\n"
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c)
        : [n] "r" (n_long)
        : "v0", "v1", "v2", "x0", "cc", "memory"
    );
}


void func_unroll_interleave(int *a, int* b, int* c, int n) {
    long n_long = n;
    asm volatile (
        "mov x0, 0\n"
        "LOOP_SIMPLE%=:\n"
        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "ld1 { v3.4s }, [%[a]], #16\n"
        "ld1 { v4.4s }, [%[b]], #16\n"
        "ld1 { v6.4s }, [%[a]], #16\n"
        "ld1 { v7.4s }, [%[b]], #16\n"
        "ld1 { v9.4s }, [%[a]], #16\n"
        "ld1 { v10.4s }, [%[b]], #16\n"

        "mul v2.4s, v0.4s, v1.4s\n"
        "mul v5.4s, v3.4s, v4.4s\n"
        "mul v8.4s, v6.4s, v7.4s\n"
        "mul v11.4s, v9.4s, v10.4s\n"

        "st1 { v2.4s }, [%[c]], #16\n"
        "add x0, x0, #16\n"
        "st1 { v5.4s }, [%[c]], #16\n"
        "cmp x0, %[n]\n"
        "st1 { v8.4s }, [%[c]], #16\n"
        "st1 { v11.4s }, [%[c]], #16\n"

        "b.ne LOOP_SIMPLE%=\n"
        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c)
        : [n] "r" (n_long)
        : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "x0", "cc", "memory"
    );
}

void func_simple_pipeline(int *a, int* b, int* c, int n) {
    long n_long = n;
    asm volatile (
        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "ld1 { v0.4s }, [%[a]], #16\n"
        "ld1 { v1.4s }, [%[b]], #16\n"

        "mov x0, 8\n"
        "LOOP_SIMPLE%=:\n"
        "st1 { v2.4s }, [%[c]], #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "add x0, x0, #4\n"
        "ld1 { v0.4s }, [%[a]], #16\n"
        "cmp x0, %[n]\n"
        "ld1 { v1.4s }, [%[b]], #16\n"
        "b.ne LOOP_SIMPLE%=\n"
        "st1 { v2.4s }, [%[c]], #16\n"
        "mul v2.4s, v0.4s, v1.4s\n"
        "st1 { v2.4s }, [%[c]], #16\n"

        : [a] "+r" (a), [b] "+r" (b), [c] "+r" (c)
        : [n] "r" (n_long)
        : "v0", "v1", "v2", "x0", "cc", "memory"
    );
}

template <typename T>
bool vectors_equal(const std::vector<T>& v1, const std::vector<T>& v2) {
    if (v1.size() != v2.size()) {
        return false;
    }

    auto n = v1.size();
    for (int i = 0; i < n; i++) {
        if (v1[i] != v2[i]) {
            return false;
        }
    }
    return true;
}


// Weird number to avoid cache conflicts
static constexpr int SIZE = 64*1024*1024 - 5*1024;

int main(int argc, char** argv) {
   std::vector<int> in1(SIZE);
   std::vector<int> in2(SIZE);
   std::vector<int> out_ref(SIZE);

   LIKWID_MARKER_INIT;

   std::iota(in1.begin(), in1.end(), 0);
   std::random_shuffle(in1.begin(), in1.end());
   std::iota(in2.begin(), in2.end(), 0);
   std::random_shuffle(in2.begin(), in2.end());
    
   // Warm up
   func_simple(in1.data(), in2.data(), out_ref.data(), SIZE);
   
   LIKWID_MARKER_START("simple");
   func_simple(in1.data(), in2.data(), out_ref.data(), SIZE);
   LIKWID_MARKER_STOP("simple");

   {
      std::vector<int> out(SIZE);
      LIKWID_MARKER_START("intrinsics");
      func_simple_intrinsics(in1.data(), in2.data(), out.data(), SIZE);
      LIKWID_MARKER_STOP("intrinsics");

      if (!vectors_equal(out, out_ref)) {
        std::cout << "Not same\n";
      }
   }

   {
      std::vector<int> out(SIZE);
      LIKWID_MARKER_START("simple_asm");
      func_simple_asm(in1.data(), in2.data(), out.data(), SIZE);
      LIKWID_MARKER_STOP("simple_asm");

      if (!vectors_equal(out, out_ref)) {
        std::cout << "Not same asm\n";
      }
   }

   {
      std::vector<int> out(SIZE);
      LIKWID_MARKER_START("unroll");
      func_simple_unroll(in1.data(), in2.data(), out.data(), SIZE);
      LIKWID_MARKER_STOP("unroll");

      if (!vectors_equal(out, out_ref)) {
        std::cout << "Not same unroll\n";
      }
   }

   {
      std::vector<int> out(SIZE);
      LIKWID_MARKER_START("interleave");
      func_unroll_interleave(in1.data(), in2.data(), out.data(), SIZE);
      LIKWID_MARKER_STOP("interleave");

      if (!vectors_equal(out, out_ref)) {
        std::cout << "Not same interleave\n";
      }
   }

   {
      std::vector<int> out(SIZE);
      LIKWID_MARKER_START("pipeline");
      func_simple_pipeline(in1.data(), in2.data(), out.data(), SIZE);
      LIKWID_MARKER_STOP("pipeline");

      if (!vectors_equal(out, out_ref)) {
        std::cout << "Not same pipeline\n";
      } else {
        std::cout << "Same pipeline\n";
      }
   }

   LIKWID_MARKER_CLOSE;
}
