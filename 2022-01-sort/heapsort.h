// Source: https://www.geeksforgeeks.org/heap-sort/

template <typename T>
void heapify(std::vector<T>& vec, int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    if (l < n && vec[l] > vec[largest])
        largest = l;

    if (r < n && vec[r] > vec[largest])
        largest = r;

    if (largest != i) {
        std::swap(vec[i], vec[largest]);

        heapify(vec, n, largest);
    }
}

template <typename T>
void heapsort(std::vector<T>& vec) {

    int n = vec.size();

    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(vec, n, i);

    for (int i = n - 1; i > 0; i--) {
        std::swap(vec[0], vec[i]);

        heapify(vec, i, 0);
    }
}


template <typename T>
void heapify2(std::vector<T>& vec, int n, int i) {
    int largest = i;

    int start = 4 * i + 1;
    int end = std::min(start + 4, n);

    for (int j = start; j < end; j++) {
        if (vec[j] > vec[largest]) {
            largest = j;
        }
    }

    if (largest != i) {
        std::swap(vec[i], vec[largest]);

        heapify2(vec, n, largest);
    }
}

/*__asm__ (
    "ucomiss %[vec_j], %[vec_largest];"
    "cmovbe %[j], %[largest];"
    : [largest] "+&r"(largest)
    : [vec_j] "m"(vec_j), [vec_largest] "x"(vec_largest), [j] "r" (j) 
    : "cc"
);*/


template <typename T>
void heapsort2(std::vector<T>& vec) {

    int n = vec.size();

    for (int i = n / 4 - 1; i >= 0; i--)
        heapify2(vec, n, i);

    for (int i = n - 1; i > 0; i--) {
        std::swap(vec[0], vec[i]);

        heapify2(vec, i, 0);
    }
}