#include "../common/argparse.h"
#include <immintrin.h>
#include <cstring>
#include <likwid.h>
#include <limits>

static constexpr size_t NOT_FOUND = std::numeric_limits<size_t>::max();

std::vector<size_t> data_lookup(const std::vector<int64_t> & lookup_vector, const std::vector<int64_t> & lookup_vals) {
    std::vector<size_t> result(lookup_vals.size(), NOT_FOUND);

    for (size_t i = 0; i < lookup_vals.size(); i++) {
        int64_t val = lookup_vals[i];

        for (size_t j = 0; j < lookup_vector.size(); j++) {
            if (val == lookup_vector[j]) {
                result[i] = j;
                break;
            }
        }
    }

    return result;
}

std::vector<size_t> data_lookup_vectorized(const std::vector<int64_t> & lookup_vector, const std::vector<int64_t> & lookup_vals) {
    std::vector<size_t> result(lookup_vals.size(), NOT_FOUND);
    size_t* result_ptr = result.data();
    int64_t const * const lookup_vector_ptr = lookup_vector.data();

    size_t lookup_vector_size = lookup_vector.size();
    size_t lookup_vector_size_vector_end = lookup_vector_size / 4 * 4;
    size_t lookup_vals_size = lookup_vals.size();

    for (size_t i = 0; i < lookup_vals_size; i++) {
        __m256i val = _mm256_set1_epi64x(lookup_vals[i]);

        for (size_t j = 0; j < lookup_vector_size_vector_end; j+=4) {
            __m256i lookup_vector_val = _mm256_loadu_si256((__m256i*) (lookup_vector_ptr + j));
            __m256i cmp_mask = _mm256_cmpeq_epi64(lookup_vector_val, val);
            int cmp_bitmask = _mm256_movemask_epi8(cmp_mask);
            if (cmp_bitmask != 0) {
                for (size_t k = 0; k < 4; k++) {
                    if (lookup_vals[i] == lookup_vector[j + k]) {
                        result[i] = j + k;
                        goto skip_loop;
                    }
                }
            }
        }

        for (size_t j = lookup_vector_size_vector_end; j < lookup_vector.size(); j++) {
            if (lookup_vals[i] == lookup_vector[j]) {
                result[i] = j;
                break;
            }
        }

        skip_loop: ;
    }

    return result;
}

std::vector<size_t> data_lookup2(const std::vector<int64_t> & lookup_vector, const std::vector<int64_t> & lookup_vals) {
    std::vector<size_t> result(lookup_vals.size(), NOT_FOUND);
    size_t values_found = 0;

    for (size_t i = 0; i < lookup_vector.size(); i++) {
        int64_t val = lookup_vector[i];

        if (values_found >= lookup_vals.size()) {
            break;
        }

        for (size_t j = 0; j < lookup_vals.size(); j++) {
            if (val == lookup_vals[j]) {
                result[j] = i;
                // We don't break the loop because lookup_vals
                // array is fine if it contain duplicates
                values_found++;
            }
        }
    }

    return result;
}

std::vector<size_t> data_lookup2_vectorized(const std::vector<int64_t> & lookup_vector, const std::vector<int64_t> & lookup_vals) {
    std::vector<size_t> result(lookup_vals.size(), NOT_FOUND);
    int64_t const * const lookup_vals_ptr = lookup_vals.data();

    size_t lookup_vals_size = lookup_vals.size();
    size_t lookup_vals_vector_size = lookup_vals_size / 4 * 4;

    size_t values_found = 0;

    for (size_t i = 0; i < lookup_vector.size(); i++) {
        __m256i val = _mm256_set1_epi64x(lookup_vector[i]);

        if (values_found >= lookup_vals_size) {
            break;
        }

        for (size_t j = 0; j < lookup_vals_vector_size; j+=4) {
            __m256i lookup_val = _mm256_loadu_si256((__m256i*) (lookup_vals_ptr + j));
            __m256i cmp_mask = _mm256_cmpeq_epi64(val, lookup_val);
            int cmp_bitmask = _mm256_movemask_epi8(cmp_mask);
            if (cmp_bitmask != 0) {
                for (size_t k = 0; k < 4; k++) {
                    if (lookup_vector[i] == lookup_vals[j + k]) {
                        result[j + k] = i;
                        values_found++;
                    }
                }
            }
        }

        for (size_t j = lookup_vals_vector_size; j < lookup_vals_size; j++) {
            if (lookup_vector[i] == lookup_vals[j]) {
                result[j] = i;
                // We don't break the loop because lookup_vals
                // array is fine if it contain duplicates
            }
        }
    }

    return result;
}


template <typename T>
bool buffers_same(const T& a, const T& b) {
    if (a.size() != b.size()) {
        std::cout << "Different buffer sizes\n";
        return false;
    }
    for (size_t i = 0; i < a.size(); i++) {
        if (a[i] != b[i]) {
            std::cout << "Buffers different at position " << i << "\n";
            std::cout << "val1 " << a[i] << ", val2 " << b[i] << "\n";
            return false;
        }
    }
    std::cout << "Buffers same\n";
    return true;
}

void verify_result(const std::vector<int64_t>& lookup_vector, const std::vector<int64_t>& lookup_vals, const std::vector<size_t>& res) {
    if (res.size() != lookup_vals.size()) {
        std::cout << "Size mismatch\n";
        return;
    }

    for (size_t i = 0; i < res.size(); i++) {
        if (res[i] != NOT_FOUND) {
            if (lookup_vals[i] != lookup_vector[res[i]]) {
                std::cout << "Results not correct at position " << i << "\n";
                return;
            }
        }
    }
    std::cout << "Results correct\n";
}

static void clobber() {
    asm volatile("" : : : "memory");
}


using namespace argparse;

int main(int argc, const char* argv[]) {
    ArgumentParser parser("matmul", "matmul");

    parser.add_argument("-a", "--array", "Size of the array we perform lookup in", true);
    parser.add_argument("-v", "--values", "Size of the array we want to lookup in -a", true);

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return 0;
    }

    size_t lookup_vals_size = parser.get<size_t>("v");
    size_t lookup_vector_size = parser.get<size_t>("a");
    size_t repeat_count = std::max<size_t>(100000000ULL / (lookup_vals_size * lookup_vector_size), 1);

    std::cout << "Lookup vector size " << lookup_vector_size << ", lookup vals size " << lookup_vals_size
              << ", repeat count " << repeat_count << std::endl;

    std::vector<int64_t> tmp(std::max(lookup_vector_size, lookup_vals_size)  * 2);
    std::iota(tmp.begin(), tmp.end(), 0);
    std::random_shuffle(tmp.begin(), tmp.end());

    std::vector<int64_t> lookup_vector(tmp.begin(), tmp.begin() + lookup_vector_size);

    std::random_shuffle(tmp.begin(), tmp.end());
    std::vector<int64_t> lookup_vals(tmp.begin(), tmp.begin() + lookup_vals_size);


    LIKWID_MARKER_INIT;

    std::vector<size_t> res0, res1;

    LIKWID_MARKER_START("values_in_vector");
    for (size_t i = 0; i < repeat_count; i++) {
        res0 = data_lookup_vectorized(lookup_vector, lookup_vals);
        clobber();
    }
    LIKWID_MARKER_STOP("values_in_vector");

    LIKWID_MARKER_START("vector_in_values");
    for (size_t i = 0; i < repeat_count; i++) {
        res1 = data_lookup2_vectorized(lookup_vector, lookup_vals);
        clobber();
    }
    LIKWID_MARKER_STOP("vector_in_values");

    verify_result(lookup_vector, lookup_vals, res0);
    verify_result(lookup_vector, lookup_vals, res1);
    buffers_same(res0, res1);

    LIKWID_MARKER_CLOSE;
}