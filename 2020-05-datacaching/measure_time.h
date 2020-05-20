#include <chrono>
#include <string>
#include <iostream>
#include <map>


template <typename T>
class measure_time_database {
public:
    static measure_time_database* get_instance() {
        if (!my_instance) {
            my_instance = new measure_time_database;
        }
        return my_instance;
    }

    void set_result(std::string& measure_message, T duration) {
        my_measurements[measure_message].set_result(duration);
    }

    void dump_database() {
        for(auto const& m: my_measurements) {
            std::cout << "measurement|" << m.first << "|" << m.second.get_average_time() << "ms\n";
        }
    }

private:
    class measurement {
        T all_time;
        int num_iterations;
    public:
        void set_result(T res) {
            all_time = all_time + res;
            num_iterations++;
        }

        int get_average_time() const {
            return all_time / num_iterations / std::chrono::milliseconds(1);
        }  
    };

    static measure_time_database* my_instance;
    std::map<std::string, measurement> my_measurements;
};


template <typename T>
measure_time_database<T>* measure_time_database<T>::my_instance;


class measure_time {
public:

    measure_time(const std::string& message) : message_(message) {
        start_time = std::chrono::high_resolution_clock::now();
        
        std::cout << "Starting measurement for \"" << message_ << "\"\n";
    }

    ~measure_time() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "\"" << message_ << "\" took " << time /std::chrono::milliseconds(1) << "ms to run.\n";
        measure_time_database<std::chrono::milliseconds>::get_instance()->set_result(message_, time);
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::string message_;
};
