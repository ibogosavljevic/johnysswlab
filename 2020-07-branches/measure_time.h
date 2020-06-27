#include <chrono>
#include <string>
#include <iostream>
#include <map>
#include <papi.h>


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

#define array_length(x) (sizeof(x) / sizeof(x[0]))

class measure_time {
public:

    measure_time(const std::string& message) : message_(message) {
        
        std::cout << "Starting measurement for \"" << message_ << "\"\n";

        start_time = std::chrono::high_resolution_clock::now();
        int ret_val = PAPI_start_counters(events, events_length);
        papi_valid = ret_val == PAPI_OK;        
        asm volatile("": : :"memory");
    }

    ~measure_time() {
        asm volatile("": : :"memory");

        auto end_time = std::chrono::high_resolution_clock::now();
        long_long values[array_length(events)];

        if (papi_valid) {
            papi_valid = PAPI_stop_counters(values, events_length) == PAPI_OK; 
        } else {
            std::cout << "PAPI not valid\n";
        }

        
        std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "\"" << message_ << "\" took " << time /std::chrono::milliseconds(1) << "ms to run.\n";
        if (papi_valid) {
            std::cout << "\t" << "L1 Cache missess: " << values[0] << "\n";
            std::cout << "\t" << "L1 Cache accesses: " << values[1] << "\n";
            std::cout << "\t" << "L1 Cache miss rate: " << values[0] * 100.0 / values[1] << "%\n";
        }
        measure_time_database<std::chrono::milliseconds>::get_instance()->set_result(message_, time);
    }
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::string message_;
    bool papi_valid;

    static constexpr int events_length = 2;

    int events[events_length] = {PAPI_L1_DCM, PAPI_L1_DCA };
};
