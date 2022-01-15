// Source: https://www.geeksforgeeks.org/quick-sort/

template <typename T>
void print_array(std::vector<T>& vec) {
    for(const auto& v: vec) {
        std::cout << v << ", ";
    }
    std::cout << std::endl;
}

template <typename T>
void print_array2(std::vector<T>& vec, int low, int high) {
    for(int i = low; i <= high; i++) {
        std::cout << vec[i] << ", ";
    }
    std::cout << std::endl;
}


template <typename T>
int partition(std::vector<T>& vector, int low, int high) {
    T pivot = vector[high];

    int i = (low - 1);

    for (int j = low; j < high; j++) {
        if (vector[j] <= pivot) {
            i++;
            std::swap(vector[i], vector[j]);
        }
    }
    i = i + 1;
    std::swap(vector[i], vector[high]);
    return i;
}

template <typename T>
void quicksort_internal(std::vector<T>& vector, int low, int high) {
    if (low < high) {
        int pi = partition(vector, low, high);

        quicksort_internal(vector, low, pi - 1);
        quicksort_internal(vector, pi + 1, high);
    }
}

template <typename T>
void quicksort(std::vector<T>& vector) {
    quicksort_internal(vector, 0, vector.size() - 1);
}
