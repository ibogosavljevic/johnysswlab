#ifdef __AVX__
#include <immintrin.h>
#endif
#include <cassert>
#include <iostream>

template <typename T>
class matrix {
   public:
    matrix(int rows, int cols)
        : m_data(allocate_matrix(rows, cols)), m_rows(rows), m_cols(cols) {}

    ~matrix() { free_matrix(m_data, m_rows, m_cols); }

    matrix(const matrix& m) = delete;

    static bool multiply_naive(matrix& out, matrix& in1, matrix& in2) {
        T** c = out.m_data;
        T** a = in1.m_data;
        T** b = in2.m_data;
        int n;

        if (!verify_multiplication_params(out, in1, in2)) {
            return false;
        }

        n = out.m_rows;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                c[i][j] = 0;
                for (int k = 0; k < n; k++) {
                    c[i][j] = c[i][j] + a[i][k] * b[k][j];
                }
            }
        }

        return true;
    }

    static bool multiply_interchanged(matrix& out, matrix& in1, matrix& in2) {
        T** c = out.m_data;
        T** a = in1.m_data;
        T** b = in2.m_data;
        int n;

        if (!verify_multiplication_params(out, in1, in2)) {
            return false;
        }

        n = out.m_rows;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                c[i][j] = 0;
            }
        }
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                for (int j = 0; j < n; j++) {
                    c[i][j] = c[i][j] + a[i][k] * b[k][j];
                }
            }
        }

        return true;
    }

    template <int tile_size>
    static bool multiply_tiled(matrix& out, matrix& in1, matrix& in2) {
        T** c = out.m_data;
        T** a = in1.m_data;
        T** b = in2.m_data;
        int n;

        if (!verify_multiplication_params(out, in1, in2)) {
            return false;
        }

        n = out.m_rows;

        if (n % tile_size != 0) {
            return false;
        }

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                c[i][j] = 0;
            }
        }
        for (int ii = 0; ii < n; ii += tile_size) {
            for (int kk = 0; kk < n; kk += tile_size) {
                for (int jj = 0; jj < n; jj += tile_size) {
                    for (int i = ii; i < ii + tile_size; i++) {
                        for (int k = kk; k < kk + tile_size; k++) {
                            for (int j = jj; j < jj + tile_size; j++) {
                                c[i][j] = c[i][j] + a[i][k] * b[k][j];
                            }
                        }
                    }
                }
            }
        }

        return true;
    }

    static bool multiply_tiled_avx(matrix& out,
                                   matrix& in1,
                                   matrix& in2,
                                   int tile_size) {
        assert(false);
        return false;
    }

    int rows() { return m_rows; }
    int cols() { return m_cols; }

    T& operator()(int row, int col) { return m_data[row][col]; }

    bool operator==(const matrix& m) {
        if (m.m_rows != m_rows)
            return false;
        if (m.m_cols != m_cols)
            return false;

        for (int i = 0; i < m_rows; i++) {
            for (int j = 0; j < m_cols; j++) {
                if (m.m_data[i][j] != m_data[i][j])
                    return false;
            }
        }

        return true;
    }

    bool operator!=(const matrix& m) { return !(*this == m); }

   private:
    T** m_data;
    int m_rows;
    int m_cols;

    static T** allocate_matrix(int rows, int cols) {
        T** matrix = new T*[rows];
        if (rows) {
            matrix[0] = new T[rows * cols];
            for (int i = 1; i < rows; ++i)
                matrix[i] = matrix[0] + i * cols;
        }

        return matrix;
    }

    static void free_matrix(T** matrix, int rows, int cols) {
        if (rows)
            delete[] matrix[0];
        delete[] matrix;
    }

    static bool verify_multiplication_params(const matrix& out1,
                                             const matrix& in1,
                                             const matrix& in2) {
        if (out1.m_rows != out1.m_cols)
            return false;
        if (out1.m_rows != in1.m_rows || out1.m_cols != in1.m_cols)
            return false;
        if (out1.m_rows != in2.m_rows || out1.m_cols != in2.m_cols)
            return false;
        return true;
    }

    template <typename Q>
    friend std::ostream& operator<<(std::ostream& os, const matrix<Q>& m);
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const matrix<T>& m) {
    for (int i = 0; i < m.m_rows; i++) {
        for (int j = 0; j < m.m_cols; j++) {
            os << m.m_data[i][j] << " ";
        }
        os << "\n";
    }
    return os;
}

#ifdef __AVX__

template <>
bool matrix<double>::multiply_tiled_avx(matrix<double>& out,
                                        matrix<double>& in1,
                                        matrix<double>& in2,
                                        int tile_size) {
    double** c = out.m_data;
    double** a = in1.m_data;
    double** b = in2.m_data;
    int n;

    if (!verify_multiplication_params(out, in1, in2)) {
        return false;
    }

    n = out.m_rows;

    if (n % tile_size != 0) {
        return false;
    }

    if (tile_size % 4 != 0) {
        return false;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            c[i][j] = 0;
        }
    }

    for (int ii = 0; ii < n; ii += tile_size) {
        for (int kk = 0; kk < n; kk += tile_size) {
            for (int jj = 0; jj < n; jj += tile_size) {
                for (int i = ii; i < ii + tile_size; i++) {
                    double* c_i = c[i];
                    double* a_i = a[i];
                    for (int k = kk; k < kk + tile_size; k++) {
                        double* b_k = b[k];
                        __m256d a_i_k = _mm256_set1_pd(a_i[k]);
                        /*for (int j = jj; j < jj + tile_size; j++) {
                            c_i[j] = c_i[j] + a_i[k] * b_k[j];
                        }*/

                        for (int j = jj; j < jj + tile_size; j += 4) {
                            __m256d c_i_j = _mm256_loadu_pd(c_i + j);
                            __m256d b_k_j = _mm256_loadu_pd(b_k + j);
                            c_i_j = _mm256_fmadd_pd(a_i_k, b_k_j, c_i_j);
                            _mm256_storeu_pd(c_i + j, c_i_j);
                        }
                    }
                }
            }
        }
    }

    return true;
}

#endif
