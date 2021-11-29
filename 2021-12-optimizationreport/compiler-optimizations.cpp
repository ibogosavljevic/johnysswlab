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

void test_vectorization(int* a, int* b, int* c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + c[i];
    }
}

int test_vectorization_reduction(int* a, int n) {
    int sum = 0;
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

void test_distribution(int *a, int *b, int* c, int* e, int n) {
    #pragma clang loop distribute(enable)
    for (int i = 0; i < n; i++) {
        a[i] = a[i - 1] * b[i];
        c[i] = a[i] + e[i];
    }
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