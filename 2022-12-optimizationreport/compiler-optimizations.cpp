#include <utility>

void test_spilling(int* a, int* b, int* c, int *x, int* y, int* z, int n) {
    for (int i = 0; i< n; i++) {
        a[i] = a[i] + i;
        b[i] = b[i] + i;
        c[i] = c[i] + i;
        x[i] = x[i] + i;
        y[i] = y[i] + i;
        z[i] = z[i] + i;
  } 
}

void test_pointer_aliasing(int* a, int* b, int* c, int *x, int* y, int* z, int n) {
    for (int i = 0; i< n; i++) {
        a[i] = a[i] + i;
        b[i] = b[i] + i;
        c[i] = c[i] + i;
        x[i] = x[i] + i;
        y[i] = y[i] + i;
        z[i] = a[i] + b[i] + c[i] + x[i] + y[i] + z[i];
  } 
}

void test_pointer_aliasing_fix(int* __restrict a, int* __restrict  b, int* __restrict  c, int * __restrict x, int* __restrict  y, int* __restrict z, int n) {
    for (int i = 0; i< n; i++) {
        a[i] = a[i] + i;
        b[i] = b[i] + i;
        c[i] = c[i] + i;
        x[i] = x[i] + i;
        y[i] = y[i] + i;
        z[i] = a[i] + b[i] + c[i] + x[i] + y[i] + z[i];
  } 
}


double f1(double* a, int i, int pivot) {
    if (a[i] > pivot) {
        return a[i];
    }
}

void f2(double* a, int i, int pivot) {
    if (a[i] > pivot) {
        a[i]++;
    }
}

char f3(char* a, int i, int pivot) {
    if (a[i] > pivot) {
        return a[i];
    }
}

void f4(char* a, int i, int pivot) {
    if (a[i] > pivot) {
        a[i]++;
    }
}

void add(int* a, int* b) {
    (*a)++;
    if (b) (*b)++;
}


void test_dead_code_elimination(int* a, int n) {
    for (int i = 0; i < n; i++) {
        add(&a[i], nullptr);
    }
}

enum operation_e {
    ADD,
    SUB
};

void test_invariant_code_motion(int* a, int n, int x, operation_e operation) {
    for (int i = 0; i < n; i++) {
        switch (operation) {
            case ADD: a[i]+= x * x; break;
            case SUB: a[i]-= x * x; break;
        }
    }
}

void log_error(const char* err);

int test_loop_unswitching(char* a, int n, bool& debug) {
    int error_cnt = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] != 0) {
            if (debug) {
                log_error("error");
            }
            error_cnt++;
        }
    }
    return error_cnt;
}


int test_loop_unswitching_fixed(char* a, int n, bool& debug) {
    int error_cnt = 0;
    bool debug_priv = debug;
    for (int i = 0; i < n; i++) {
        if (a[i] != 0) {
            if (debug_priv) {
                log_error("error");
            }
            error_cnt++;
        }
    }
    return error_cnt;
}

void test_licm(double* a, double* b, double* c, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                c[i * n + j] += a[i * n + k] * b[k * n + j];
            }
        }
    }
}

void test_licm_fixed(double* a, double* b, double* c, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            auto tmp = c[i * n + j];
            for (int k = 0; k < n; k++) {
                tmp += a[i * n + k] * b[k * n + j];
            }
            c[i * n + j] = tmp;
        }
    }
}

void test_gvn_basic(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        if ((a[i] + 1) > 0) {
            b[i] = a[i] + 1;
        }
    }
}

void test_gvn(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i+1] > 0) {
            b[i]++;
            a[i+1]++;
        }
    }
}

void test_gvn_fixed(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        auto a_val = a[i + 1];
        if (a_val > 0) {
            b[i]++;
            a[i+1] = a_val + 1;
        }
    }
}

void test_gvn1(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {
            a[i]++;
        } else {
            a[i]--;
        }
    }
}

void test_gvn2(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = a[i] + 1;
    }
}

void test_licm(int * val, int n, int* pivot) {
    for (int i = 0; i < n; i++) {
        if (val[i] > *pivot) {
            val[i]++;
        }
    }
}

void f(int* p);

void test_licm2(int * val, int n, int pivot) {
    f(&pivot);
    for (int i = 0; i < n; i++) {
        if (val[i] > pivot) {
            val[i]++;
        }
    }
}

struct string {
    char* p;
    int n;
};

void test_pointeraliasing(string* str) {
    for (int i = 0; i < str->n; i++) {
        str->p[i] = '\0';
    }
}

void test_pointeraliasing_fixed(string* str) {
    int n = str->n;
    char* p = str->p;
    for (int i = 0; i < n; i++) {
        p[i] = '\0';
    }
}

void test_pointeraliasing2(int * v1, int* v2, int n) {
    for (int i = 0; i < n; i++) {
        v1[i] = v2[2*i];
    }
}

void test_induction_variables(int* a, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
    }
}

void test_loop_unrolling(int* a, int * b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = b[i/2];
    }
}

void test_loop_unrolling_fixed(int* a, int * b, int n) {
    for (int i = 0; i < n/2; i++) {
        a[2 * i] = b[i];
        a[2 * i+1] = b[i];
    }
}

void test_vectorization(int* a, int* b, int* c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + c[i];
    }
}

double test_vectorization_reduction(double* a, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += a[i];
    }

    return sum;
}

struct complex { double re; double im; };

complex square(complex in) {
    complex result;
    result.re = in.re * in.re - in.im * in.im;
    result.im = 2.0 * in.re * in.im;
    return result;
}

void test_interleaved_data(complex* a, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = square(a[i]);
    }
}

void test_interchange(int* a, int* b, int n) {
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < n; j++) {
            b[j * n + i] = a[j * n + i] - a[(j - 1)* n + i];
        }
    }
}

void test_interchange_fixed(int* a, int* b, int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 1; i < n; i++) {
            b[j * n + i] = a[j * n + i] - a[(j - 1)* n + i];
        }
    }
}

void test_distribution(int *a, int *b, int* c, int* e, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = a[i - 1] * b[i];
        c[i] = a[i] + e[i];
    }
}

void test_distribution_fixed(int * __restrict a, int * __restrict b, int* __restrict c, int* __restrict e, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = a[i - 1] * b[i];
        c[i] = a[i] + e[i];
    }
}


void test_partitioning(int* v, int pivot, int n) {
    int i = -1;

    for (int j = 0; j < n; j++) {
        if (v[j] <= pivot) {
            i++;
            std::swap(v[i], v[j]);
        }
    }
    i = i + 1;
    std::swap(v[i], v[n-1]);
}

void test_fusion(int* a, int n, int* out_max, int* out_min) {
    // Find minimum loop
    auto min = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] < min) min = a[i];
    }
    // Find maximum loop
    auto max = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] > max) max = a[i];
    }

    *out_max = max;
    *out_min = min;
}

void test_gvn3(double* a, double *b, int n) {
    for (int i = 0; i < n; ++i) {
        b[i] = a[i] + 1;
        if (a[i] > 0) {
            a[i] += 2;
        }
    }
}