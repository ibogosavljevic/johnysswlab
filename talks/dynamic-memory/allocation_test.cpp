#include <set>
#include <string>
#include <vector>

#include <unistd.h>
#include <ctime>
#include <iostream>

std::string gen_random(const int max_len) {
    int len = rand() % max_len;
    std::string tmp_s;
    tmp_s.reserve(len);
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    srand((unsigned)time(NULL) * getpid());

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

    return tmp_s;
}

struct subject {
    std::string name;
    subject(std::string my_name) : name(my_name) {}
};

struct student {
    int id;
    std::string name;
    std::vector<subject*> subjects;
};

struct professor {
    std::string name;
    std::vector<subject*> subjects;
};

bool operator<(const professor& p1, const professor& p2) {
    return p1.name < p2.name;
}

bool operator<(const student& s1, const student& s2) {
    if (s1.id != s2.id) {
        return s1.id < s2.id;
    }
    return s1.name < s2.name;
}

int main(int argc, char* argv[]) {
    std::vector<subject> subjects;

    std::set<student> students;
    std::set<professor> professors;
    std::vector<student> student_array;
    std::vector<professor> professor_array;

    for (int i = 0; i < 10000; i++) {
        subjects.emplace_back(gen_random(30));
    }

    for (int k = 0; k < 10000; k++) {
        // Insert professors and students
        for (int i = 0; i < 100; i++) {
            // Generate a random professor
            professor p;
            p.name = gen_random(30);

            int subject_count = rand() % 30;
            for (int j = 0; j < subject_count; j++) {
                int id = rand() % subjects.size();
                p.subjects.emplace_back(&subjects[id]);
            }

            professors.insert(p);
            professor_array.push_back(p);

            // Generate a random student
            student s;
            s.name = gen_random(30);
            s.id = i;

            subject_count = rand() % 30;
            for (int j = 0; j < subject_count; j++) {
                int id = rand() % subjects.size();
                s.subjects.emplace_back(&subjects[id]);
            }

            students.insert(s);
            student_array.push_back(s);
        }

        // Remove professors and students
        for (int i = 0; i < 60; i++) {
            students.erase(student_array.back());
            professors.erase(professor_array.back());

            student_array.pop_back();
            professor_array.pop_back();
        }
    }

    std::cout << "Done\n";
}
