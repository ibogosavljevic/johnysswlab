#include <chrono>
#include <string>
#include <iostream>
#include <map>

#ifdef HAS_PAPI
#include <papi.h>
#endif

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
        std::cout << "Starting measurement for \"" << message_ << "\"\n";

        start_time = std::chrono::high_resolution_clock::now();
#if HAS_PAPI
        int events[events_length];
        load_events(events, events_length);

        int ret_val = PAPI_start_counters(events, events_length);
        papi_valid = ret_val == PAPI_OK;
#endif
        asm volatile("": : :"memory");
    }

    ~measure_time() {
        asm volatile("": : :"memory");

        auto end_time = std::chrono::high_resolution_clock::now();

#if HAS_PAPI
        long_long values[events_length];
        if (papi_valid) {
            papi_valid = PAPI_stop_counters(values, events_length) == PAPI_OK;
            if (!papi_valid) {
                std::cout << "PAPI not valid\n";
            } 
        }
#endif
        
        std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "\"" << message_ << "\" took " << time /std::chrono::milliseconds(1) << "ms to run.\n";
#if HAS_PAPI
        if (papi_valid) {
            std::cout << "\t" << "Total instructions: " << values[0] << std::endl;
            std::cout << "\t" << "Total cycles: " << values[1] << std::endl; 
            std::cout << "\t" << "Instruction per cycle: " << values[0] / (float) values[1] << std::endl;
            print_dcache(values);
            print_branches(values);
        }
#endif
        measure_time_database<std::chrono::milliseconds>::get_instance()->set_result(message_, time);
    }

#if HAS_PAPI
    static void initialize(int* events) {
        static bool initialized = false;

        if (initialized) {
            return;
        }

        initialized = true;

        char * measure_string_str = std::getenv("MEASURE_FLAGS");
        events[0] = PAPI_TOT_INS;
        events[1] = PAPI_TOT_CYC;
        events[2] = PAPI_L1_DCM;
        events[3] = PAPI_L1_DCA;
        if (measure_string_str) { 
            std::string measure_flags(measure_string_str);
            if (measure_flags == "DCACHE") {
                get_what_to_measure() = what_to_measure::DCACHE;
                events[2] = PAPI_L1_DCM;
                events[3] = PAPI_L1_DCA;
            } else if (measure_flags == "BRANCH") {
                get_what_to_measure() = what_to_measure::BRANCHES;
                events[2] = PAPI_BR_MSP;
                events[3] = PAPI_BR_TKN;
            }
        }
    }


    void print_dcache(long_long values[]) {
        if (get_what_to_measure() != what_to_measure::DCACHE) {
            return;
        }

        std::cout << "\t" << "L1 Cache missess: " << values[2] << "\n";
        std::cout << "\t" << "L1 Cache accesses: " << values[3] << "\n";
        std::cout << "\t" << "L1 Cache miss rate: " << values[2] * 100.0 / values[3] << "%\n";
    }

    void print_branches(long_long values[]) {
        if (get_what_to_measure() != what_to_measure::BRANCHES) {
            return;
        }

        std::cout << "\t" << "Branches missprediced : " << values[2] << "\n";
        std::cout << "\t" << "Total branches: " << values[3] << "\n";
        std::cout << "\t" << "Branch missprediction rate: " << (values[2] * 100.0) / values[3] << "%\n";
    }
#endif


private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::string message_;
    bool papi_valid;

    static constexpr int events_length = 4;

    enum what_to_measure {
        DEFAULT,
        DCACHE,
        BRANCHES,
    };

    static what_to_measure& get_what_to_measure() {
        static what_to_measure wtm;
        return wtm; 
    }

#if HAS_PAPI
    static int* get_events_array(){
        static int events[events_length];
        initialize(events);
        return events;
    }

    void load_events(int* events_array, int len) {
        int* static_events = get_events_array();
        for (int i = 0; i < len; i++) {
            events_array[i] = static_events[i];
        }
    }
#endif
};
