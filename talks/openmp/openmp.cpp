
#include "omp.h"
#include "likwid.h"
#include <vector>
#include <random>
#include <iostream>
#include <cassert>
#include <limits>


void add_scalar(float* c, float * a, float * b, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

void add_naive(float* c, float * a, float * b, size_t n) {
    size_t chunk_size;
    size_t i, i_start, i_end;
   
    #pragma omp parallel shared(n, a, b, c, std::cout) private (chunk_size, i, i_start, i_end) default(none)
    {
        chunk_size = n / omp_get_num_threads();

        i_start = chunk_size * omp_get_thread_num();
        i_end = chunk_size * (omp_get_thread_num() + 1);

        // Last thread
        if (i_end == chunk_size * omp_get_num_threads()) {
            i_end = n;
        }

        for (i = i_start; i < i_end; i++) {
            c[i] = a[i] + b[i];
        }
    
    }
}

void add_openmp(float* c, float * a, float * b, size_t n) {
    #pragma omp parallel for shared(a,b,c,n) default(none)
    for (size_t i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

float sum_scalar(float* a, size_t n) {
    float sum = 0.0;
    
    for (size_t i = 0; i < n; i++) {
        sum += a[i];
    }
    return sum;
}

float sum_naive(float* a, size_t n) {
    float sum = 0.0;
    
    #pragma omp parallel for shared(a, n, sum) default(none)
    for (size_t i = 0; i < n; i++) {
        #pragma omp atomic
        sum += a[i];
    }
    return sum;
}

float sum_manual(float* a, size_t n) {
    float sum = 0;

    #pragma omp parallel shared(n, sum, a) default(none)
    {
        float sum_priv = 0;

        #pragma omp for
        for (size_t i = 0; i < n; i++) {
            sum_priv += a[i];
        }

        #pragma omp atomic
        sum += sum_priv;

    }

    return sum;
}

float sum_openmp(float* a, size_t n) {
    float sum = 0;

    #pragma omp parallel for shared(n, a) reduction(+:sum) default(none)
    for (size_t i = 0; i < n; i++) {
        sum += a[i];
    }

    return sum;
}

static constexpr size_t NOT_FOUND = 0xFFFFFFFFFFFFFFFFULL;

size_t find_scalar(float* a, size_t n, float val) {
    size_t idx = NOT_FOUND;
    
    for (size_t i = 0; i < n; i++) {
        if (a[i] == val) {
            idx = i;
            break;
        }
    }
    
    return idx;
}

size_t find_naive(float* a, size_t n, float val) {
    size_t idx = NOT_FOUND;

    #pragma omp parallel for shared(a, val, n, idx) default(none)
    for (size_t i = 0; i < n; i++) {
        if (a[i] == val) {
            #pragma omp critical
            idx = i;

            #pragma omp cancel for
        }
        #pragma omp cancellation point for
    }

    return idx;
}

size_t find_openmp(float* a, size_t n, float val) {
    size_t idx = NOT_FOUND;
    size_t SECTION_SIZE = 64+16;

    #pragma omp parallel shared(a, val, n, idx, SECTION_SIZE) default(none)
    {
        #pragma omp for
        for (size_t ii = 0; ii < n; ii+=SECTION_SIZE) {
            size_t i_end = std::min(ii + SECTION_SIZE, n);
            for (size_t i = ii; i < i_end; i++) {
                if (a[i] == val) {
                    #pragma omp critical
                    idx = i;
        
                    #pragma omp cancel for
                }
            }

            #pragma omp cancellation point for
        }
    
    }

    return idx;
}


std::vector<float> generate_random_floats(std::size_t count) {
    std::vector<float> result;
    result.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (std::size_t i = 0; i < count; ++i) {
        result.push_back(dist(gen));
    }

    return result;
}

std::vector<uint8_t> generate_random_pixels(std::size_t count) {
    std::vector<uint8_t> result;
    result.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist(0, 255);

    for (std::size_t i = 0; i < count; ++i) {
        result.push_back(dist(gen));
    }

    return result;
}

struct node_t {
    node_t* left;
    node_t* right;
    float value;
};

node_t* create_from_array(const std::vector<float>& a, int left, int right) {
    if (left > right) {
        return nullptr;
    }

    size_t mid = left + (right - left) / 2;
    node_t* node = new node_t;
    node->value = a[mid];
    node->left = create_from_array(a, left, mid - 1);
    node->right = create_from_array(a, mid + 1, right);
    return node;
}

size_t count(node_t* node) {
    size_t result = 0;
    if (node->left) {
        result += count(node->left);
    }

    if (node->right) {
        result += count(node->right);
    }

    return result + 1;
}

void destroy(node_t* v) {
    if (v->left) {
        destroy(v->left);
    }

    if (v->right) {
        destroy(v->right);
    }

    delete v;
}

float find_min_scalar(node_t const * const node) {
    float res = node->value;

    if (node->left) {
        res = std::min(res, find_min_scalar(node->left));
    }

    if (node->right) {
        res = std::min(res, find_min_scalar(node->right));
    }

    return res;
}

float find_min_naive_internal(node_t const * const node) {
    node_t const * const node_left = node->left;
    node_t const * const node_right = node->right;
    float min_left = std::numeric_limits<float>::max(), min_right = std::numeric_limits<float>::max();
    
    if (node_left) {
        #pragma omp task shared(node_left, min_left) default(none)
        min_left = find_min_naive_internal(node_left);
    }

    if (node_right) {
        #pragma omp task shared(node_right, min_right) default(none)
        min_right = find_min_naive_internal(node_right);
    }

    #pragma omp taskwait
    return std::min(min_right, std::min(min_left, node->value));
}

float find_min_naive(node_t const * const root) {
    float min;
    #pragma omp parallel firstprivate(root) shared(min) default(none)
    {
        #pragma omp single
        min = find_min_naive_internal(root);
    }

    return min;
}

float find_min_openmp_parallel(node_t const * const node, int depth) {
    node_t const * const node_left = node->left;
    node_t const * const node_right = node->right;
    float min_left = std::numeric_limits<float>::max(), min_right = std::numeric_limits<float>::max();

    if (depth < 10) {
        if (node_left) {
            #pragma omp task shared(node_left, min_left, depth) default(none)
            min_left = find_min_openmp_parallel(node_left, depth + 1);
        }

        if (node_right) {
            #pragma omp task shared(node_right, min_right, depth) default(none)
            min_right = find_min_openmp_parallel(node_right, depth + 1);
        }
        
        #pragma omp taskwait
    } else {
        min_left = min_right = find_min_scalar(node);
    }
   
    return std::min(min_right, std::min(min_left, node->value));
}

float find_min_openmp(node_t const * const root) {
    float min;
    #pragma omp parallel firstprivate(root) shared(min) default(none)
    {
        #pragma omp single
        min = find_min_openmp_parallel(root, 0);
    }

    return min;
}

void histogram_scalar(size_t * hist, uint8_t* image, size_t n) {
    for (size_t i = 0; i < n; i++) {
        hist[image[i]]++;
    }
}

void histogram_openmp(size_t hist[256], uint8_t* image, size_t n) {
    #pragma omp parallel shared(hist, image, n) default(none)
    {
        size_t hist_priv[256] {};

        #pragma omp for nowait
        for (size_t i = 0; i < n; i++) {
            hist_priv[image[i]]++;
        }

        #pragma omp critical
        for (size_t i = 0; i < 256; i++) {
            hist[i] += hist_priv[i];
        }
    }

}


template<typename T>
bool near(T a, T b, T diff) {
    return std::abs(a - b) < diff;
}

int main() {
    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "Thread count " << omp_get_num_threads() << "\n";
    }
    static constexpr size_t COUNT = 128*1024*1024;

    std::vector<float> a = generate_random_floats(COUNT);
    std::vector<float> b = generate_random_floats(COUNT);
    std::vector<float> c0(COUNT), c1(COUNT), c2(COUNT);

    LIKWID_MARKER_INIT;
    LIKWID_MARKER_THREADINIT;

    LIKWID_MARKER_START("add_scalar");
    add_scalar(c0.data(), a.data(), b.data(), COUNT);
    LIKWID_MARKER_STOP("add_scalar");

    LIKWID_MARKER_START("add_naive");
    add_naive(c1.data(), a.data(), b.data(), COUNT);
    LIKWID_MARKER_STOP("add_naive");

    LIKWID_MARKER_START("add_openmp");
    add_openmp(c2.data(), a.data(), b.data(), COUNT);
    LIKWID_MARKER_STOP("add_openmp");

    if (c1 != c0 || c2 != c0) {
        std::cout << "FAIL: Add not same\n";
    } else {
        std::cout << "SUCC: Add same\n";
    }

    LIKWID_MARKER_START("sum_scalar");
    float sum0 = sum_scalar(a.data(), COUNT);
    LIKWID_MARKER_STOP("sum_scalar");

    LIKWID_MARKER_START("sum_naive");
    float sum1 = sum_naive(a.data(), COUNT);
    LIKWID_MARKER_STOP("sum_naive");

    LIKWID_MARKER_START("sum_manual");
    float sum2 = sum_manual(a.data(), COUNT);
    LIKWID_MARKER_STOP("sum_manual");
    
    LIKWID_MARKER_START("sum_openmp");
    float sum3 = sum_openmp(a.data(), COUNT);
    LIKWID_MARKER_STOP("sum_openmp");

    a[COUNT/2] = -10.0;
    float find_val = a[COUNT/2];

    float delta = 0.01 * sum0;

    if ((!near(sum0, sum1, delta)) || (!near(sum0, sum2, delta)) || (!near(sum0, sum3, delta))) {
        std::cout << "FAIL: Sums not same\n";
        std::cout << "sum0 " << sum0 << ", sum1 " << sum1 << ", sum2 " << sum2 << ", sum3 " << sum3 << "\n";
    } else {
        std::cout << "SUCC: Sums same\n";
    }

    if (!omp_get_cancellation()) {
        std::cout << "CANCELATION NOT ENABLED\nENABLE IT WITH `export OMP_CANCELLATION=true`\n";
    }

    LIKWID_MARKER_START("find_scalar");
    size_t idx0 = find_scalar(a.data(), COUNT, find_val);
    LIKWID_MARKER_STOP("find_scalar");

    LIKWID_MARKER_START("find_naive");
    size_t idx1 = find_naive(a.data(), COUNT, find_val);
    LIKWID_MARKER_STOP("find_naive");

    LIKWID_MARKER_START("find_openmp");
    size_t idx2 = find_openmp(a.data(), COUNT, find_val);
    LIKWID_MARKER_STOP("find_openmp");

    if (a[idx0] != find_val || a[idx1] != find_val || a[idx2] != find_val) {
        std::cout << "Find not good\n";
    }

    node_t* root = create_from_array(a, 0, a.size() - 1);

    LIKWID_MARKER_START("find_min_scalar");
    float min_val0 = find_min_scalar(root);
    LIKWID_MARKER_STOP("find_min_scalar");

    LIKWID_MARKER_START("find_min_naive");
    float min_val1 = find_min_naive(root);
    LIKWID_MARKER_STOP("find_min_naive");

    LIKWID_MARKER_START("find_min_openmp");
    float min_val2 = find_min_openmp(root);
    LIKWID_MARKER_STOP("find_min_openmp");

    if (min_val0 != min_val1 || min_val0 != min_val2) {
        std::cout << "FAIL: Find_min not same values\n";
    } else {
        std::cout << "SUCC: Find_min same values\n";
    }

    destroy(root);

    std::vector<uint8_t> pixels = generate_random_pixels(COUNT);
    std::vector<size_t> histogram0(256), histogram1(256);

    LIKWID_MARKER_START("hist_scalar");
    histogram_scalar(histogram0.data(), pixels.data(), COUNT);
    LIKWID_MARKER_STOP("hist_scalar");

    LIKWID_MARKER_START("hist_openmp");
    histogram_openmp(histogram1.data(), pixels.data(), COUNT);
    LIKWID_MARKER_STOP("hist_openmp");

    if (histogram0 != histogram1) {
        std::cout << "FAIL: Histograms not same\n";
    } else {
        std::cout << "SUCC: Histograms same\n";
    }

    LIKWID_MARKER_CLOSE;

    return 0;
}