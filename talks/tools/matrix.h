#include <immintrin.h>
#include <cassert>
#include <iostream>

template <typename T>
class matrix {
   public:
    matrix(int rows, int cols)
        : m_data(allocate_matrix(rows, cols)), m_rows(rows), m_cols(cols) {}

    ~matrix() { free_matrix(m_data, m_rows, m_cols); }

    matrix(const matrix& m) = delete;

    static bool multiply_mul1(matrix& out, matrix& in1, matrix& in2) {
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

    static bool multiply_mul2(matrix& out, matrix& in1, matrix& in2) {
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

