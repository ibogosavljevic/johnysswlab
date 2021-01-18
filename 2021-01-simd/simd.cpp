#include <array>
#include <string>
#include <vector>
#include "measure_time.h"

class student {
   public:
    std::string name;
    double average_mark;
    student(std::string& name, double average)
        : name(name), average_mark(average) {}
};

double calculate_average(std::vector<student>& students) {
    double val = 0.0;
    for (int i = 0; i < students.size(); i++) {
        val += students[i].average_mark;
    }
    return val / students.size();
}

class students {
   public:
    std::vector<std::string> names;
    std::vector<double> average_marks;

    double calculate_average() {
        double val = 0.0;
        int size = average_marks.size();
        for (int i = 0; i < size; i++) {
            val += average_marks[i];
        }
        return val / average_marks.size();
    }
};

static constexpr int vector_len = 100 * 1024 * 1024;

void generate_random_names(std::vector<student>& students1,
                           students& students2) {
    students1.reserve(vector_len);
    students2.names.reserve(vector_len);
    students2.average_marks.reserve(vector_len);

    std::array<std::string, 8> names = {"John",    "Mark",   "Lucas",
                                        "Matthew", "Andrew", "Bartholomew",
                                        "Paul",    "Chris"};

    int index = 0;
    for (int i = 0; i < vector_len; i++) {
        double mark = (double)((i + index) % 8);

        students1.emplace_back(names[index], mark);
        students2.names.emplace_back(names[index]);
        students2.average_marks.emplace_back(mark);

        index++;
        if (index >= names.size()) {
            index = 0;
        }
    }
}

int main(int argc, char** argv) {
    std::vector<student> students1;
    students students2;

    generate_random_names(students1, students2);

    {
        measure_time m("student");
        double r = calculate_average(students1);
        std::cout << "Average = " << r << std::endl;
    }

    {
        measure_time m("simd student");
        double r = students2.calculate_average();
        std::cout << "Average = " << r << std::endl;
    }
    return 0;
}
