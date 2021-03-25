#include <cmath>
#include <iostream>
#include "common/argparse.h"
#include "measure_time.h"
#include "utils.h"

enum operation_e { ADD, SUBSTRACT, REPLACE };

enum math_e { SIN_COS, ADD_SUB };

operation_e operation = ADD;
math_e math = SIN_COS;

void calculate(int i, float* r1, float* r2) {
    switch (math) {
        case SIN_COS:
            *r1 = sinf(i);
            if (r2)
                *r2 = cosf(i);
            break;
        case ADD_SUB:
            *r1 = i + i;
            if (r2)
                *r2 = i - i / 2;
            break;
    }
}

__attribute__((noinline)) void calculate_noinline(int i, float* r1, float* r2) {
    switch (math) {
        case SIN_COS:
            *r1 = sinf(i);
            if (r2)
                *r2 = cosf(i);
            break;
        case ADD_SUB:
            *r1 = i + i;
            if (r2)
                *r2 = i - i / 2;
            break;
    }
}

template <math_e math>
void calculate_template(int i, float* r1, float* r2) {
    switch (math) {
        case SIN_COS:
            *r1 = sinf(i);
            if (r2)
                *r2 = cosf(i);
            break;
        case ADD_SUB:
            *r1 = i + i;
            if (r2)
                *r2 = i - i / 2;
            break;
    }
}

void calculate_array(float a[], float b[], int n, bool set_b) {
    float r1, r2;
    for (int i = 0; i < n; i++) {
        calculate(i, &r1, &r2);
        if (operation == ADD) {
            a[i] += r1;
            if (set_b) {
                b[i] += r2;
            }
        } else if (operation == SUBSTRACT) {
            a[i] -= r1;
            if (set_b) {
                b[i] -= r2;
            }
        } else /* if (operation == REPLACE) */ {
            a[i] = r1;
            if (set_b) {
                b[i] = r2;
            }
        }
    }
}

void calculate_array_noinline(float a[], float b[], int n, bool set_b) {
    float r1, r2;
    for (int i = 0; i < n; i++) {
        calculate_noinline(i, &r1, &r2);
        if (operation == ADD) {
            a[i] += r1;
            if (set_b) {
                b[i] += r2;
            }
        } else if (operation == SUBSTRACT) {
            a[i] -= r1;
            if (set_b) {
                b[i] -= r2;
            }
        } else /* if (operation == REPLACE) */ {
            a[i] = r1;
            if (set_b) {
                b[i] = r2;
            }
        }
    }
}

template <operation_e operation, bool set_b>
void calculate_array2(float a[], float b[], int n) {
    float r1, r2;
    for (int i = 0; i < n; i++) {
        calculate(i, &r1, &r2);
        if (operation == ADD) {
            a[i] += r1;
            if (set_b) {
                b[i] += r2;
            }
        } else if (operation == SUBSTRACT) {
            a[i] -= r1;
            if (set_b) {
                b[i] -= r2;
            }
        } else /* if (operation == REPLACE) */ {
            a[i] = r1;
            if (set_b) {
                b[i] = r2;
            }
        }
    }
}

template <operation_e operation>
void calculate_array3(float a[], float b[], int n, bool set_b) {
    float r1, r2;
    if (set_b) {
        for (int i = 0; i < n; i++) {
            calculate(i, &r1, &r2);
            if (operation == ADD) {
                a[i] += r1;
                b[i] += r2;
            } else if (operation == SUBSTRACT) {
                a[i] -= r1;
                b[i] -= r2;
            } else /* if (operation == REPLACE) */ {
                a[i] = r1;
                b[i] = r2;
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            calculate(i, &r1, nullptr);
            if (operation == ADD) {
                a[i] += r1;
            } else if (operation == SUBSTRACT) {
                a[i] -= r1;
            } else /* if (operation == REPLACE) */ {
                a[i] = r1;
            }
        }
    }
}

template <operation_e operation, math_e math>
void calculate_array4(float a[], float b[], int n, bool set_b) {
    float r1, r2;
    if (set_b) {
        for (int i = 0; i < n; i++) {
            calculate_template<math>(i, &r1, &r2);
            if (operation == ADD) {
                a[i] += r1;
                b[i] += r2;
            } else if (operation == SUBSTRACT) {
                a[i] -= r1;
                b[i] -= r2;
            } else /* if (operation == REPLACE) */ {
                a[i] = r1;
                b[i] = r2;
            }
        }
    } else {
        for (int i = 0; i < n; i++) {
            calculate_template<math>(i, &r1, nullptr);
            if (operation == ADD) {
                a[i] += r1;
            } else if (operation == SUBSTRACT) {
                a[i] -= r1;
            } else /* if (operation == REPLACE) */ {
                a[i] = r1;
            }
        }
    }
}

using namespace argparse;

bool parse_args(int argc,
                const char* argv[],
                operation_e& out_operation,
                bool& out_set_b,
                math_e& out_math) {
    ArgumentParser parser("test123", "123");

    parser.add_argument("-o", "--operation", "Operation (add, sub, rep)", true);
    parser.add_argument("-b", "--set_b", "Set b (0, 1)", true);
    parser.add_argument("-m", "--math",
                        "Mathematical function on single element (sin, add)");

    auto err = parser.parse(argc, argv);
    if (err) {
        std::cout << err << std::endl;
        return false;
    }

    if (parser.exists("o")) {
        std::string operation = parser.get<std::string>("o");
        if (operation == "add") {
            out_operation = ADD;
        } else if (operation == "sub") {
            out_operation = SUBSTRACT;
        } else if (operation == "rep") {
            out_operation = REPLACE;
        } else {
            std::cout << "Unknown value for --size\n";
            return false;
        }
        std::cout << "Operation : " << operation << std::endl;
    } else {
        return false;
    }

    if (parser.exists("b")) {
        out_set_b = parser.get<bool>("b");
    } else {
        return false;
    }

    if (parser.exists("m")) {
        std::string math = parser.get<std::string>("m");
        if (math == "sin") {
            out_math = SIN_COS;
        } else if (math == "add") {
            out_math = ADD_SUB;
        } else {
            std::cout << "Unsupported option for math. Allowed sin, add\n";
            return false;
        }
    } else {
        return false;
    }

    return true;
}

static constexpr int ARR_LEN = 10 * 1024 * 1024;

int main(int argc, const char* argv[]) {
    bool set_b;

    std::vector<float> a = create_random_array<float>(ARR_LEN, 0, ARR_LEN);
    std::vector<float> b = a;

    if (!parse_args(argc, argv, operation, set_b, math)) {
        std::cout << "Unknown args\n";
        return -1;
    }

    {
        measure_time m("regular implementation");
        calculate_array(&a[0], &b[0], ARR_LEN, set_b);
    }

    {
        measure_time m("regular non-inlined implementation");
        calculate_array_noinline(&a[0], &b[0], ARR_LEN, set_b);
    }

    {
        measure_time m("template implementation");
        if (set_b) {
            switch (operation) {
                case ADD:
                    calculate_array2<ADD, true>(&a[0], &b[0], ARR_LEN);
                    break;
                case SUBSTRACT:
                    calculate_array2<SUBSTRACT, true>(&a[0], &b[0], ARR_LEN);
                    break;
                case REPLACE:
                    calculate_array2<REPLACE, true>(&a[0], &b[0], ARR_LEN);
                    break;
            }
        } else {
            switch (operation) {
                case ADD:
                    calculate_array2<ADD, false>(&a[0], &b[0], ARR_LEN);
                    break;
                case SUBSTRACT:
                    calculate_array2<SUBSTRACT, false>(&a[0], &b[0], ARR_LEN);
                    break;
                case REPLACE:
                    calculate_array2<REPLACE, false>(&a[0], &b[0], ARR_LEN);
                    break;
            }
        }
    }

    {
        measure_time m("template implementation2");
        switch (operation) {
            case ADD:
                calculate_array3<ADD>(&a[0], &b[0], ARR_LEN, set_b);
                break;
            case SUBSTRACT:
                calculate_array3<SUBSTRACT>(&a[0], &b[0], ARR_LEN, set_b);
                break;
            case REPLACE:
                calculate_array3<REPLACE>(&a[0], &b[0], ARR_LEN, set_b);
                break;
        }
    }

    {
        measure_time m("template implementation3");
        switch (math) {
            case SIN_COS:
                switch (operation) {
                    case ADD:
                        calculate_array4<ADD, SIN_COS>(&a[0], &b[0], ARR_LEN,
                                                       set_b);
                        break;
                    case SUBSTRACT:
                        calculate_array4<SUBSTRACT, SIN_COS>(&a[0], &b[0],
                                                             ARR_LEN, set_b);
                        break;
                    case REPLACE:
                        calculate_array4<REPLACE, SIN_COS>(&a[0], &b[0],
                                                           ARR_LEN, set_b);
                        break;
                }
                break;
            case ADD_SUB:
                switch (operation) {
                    case ADD:
                        calculate_array4<ADD, ADD_SUB>(&a[0], &b[0], ARR_LEN,
                                                       set_b);
                        break;
                    case SUBSTRACT:
                        calculate_array4<SUBSTRACT, ADD_SUB>(&a[0], &b[0],
                                                             ARR_LEN, set_b);
                        break;
                    case REPLACE:
                        calculate_array4<REPLACE, ADD_SUB>(&a[0], &b[0],
                                                           ARR_LEN, set_b);
                        break;
                }
                break;
        }
    }
}