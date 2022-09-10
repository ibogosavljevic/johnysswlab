// Use on systems where LIKWID is not available
#include <unordered_map>
#include <map>
#include <string>
#include <time.h>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include "omp.h"


template <typename UNUSED>
class likwid_stub {
public:
    void init() { m_created_count = 0; }
    void close() {
        struct compare_obj {
            bool operator() (const measure_data_t& lhs, const measure_data_t& rhs) const { return lhs.created < rhs.created; }
	    };

        std::vector<std::pair<std::string, measure_data_t>> m_measure_data_sorted(m_measure_data.begin(), m_measure_data.end());
	    std::sort(m_measure_data_sorted.begin(), m_measure_data_sorted.end(), [](const auto& lhs, const auto& rhs) -> bool { return lhs.second.created < rhs.second.created; }); 

        for (std::pair<std::string, measure_data_t> d: m_measure_data_sorted) {
            std::string& region_name = d.first;

            int id = 0;
            std::cout << "Region " << region_name << "\n";
            for (const auto & m: d.second.thread_data) {
                std::cout << "\tCPU id = " << id << ", count = " << m.second.total_count << ", runtime " << timespec_tosec(&m.second.total_time) << "\n";
                id++;
            }

        }
    }

    void start(const char * region_name) {
        std::string str_reg_name(region_name);
        measure_data_t* my_datum = &m_measure_data[str_reg_name];

        if (my_datum->created == -1) {
            my_datum->created = m_created_count;
            m_created_count++;
        }

        cpu_id_t cpu_id = get_current_cpu();
        /*cpu_affinity_mask_t affinity_mask = get_affinity_mask();
        set_affinity_mask(cpu_id);
        yield();*/

        typename measure_data_t::thread_measure_data_t& thread_datum = my_datum->thread_data[cpu_id];

        if (thread_datum.started_time.tv_sec != 0 || thread_datum.started_time.tv_nsec != 0) {
            std::cout << "Region " << str_reg_name << " already started. Restarting\n";
        }

        clock_gettime(CLOCK_MONOTONIC, &thread_datum.started_time);
    }

    void stop(const char * region_name) {
        std::string str_reg_name(region_name);
        auto datum = m_measure_data.find(str_reg_name);

        if (datum == m_measure_data.end()) {
            std::cout << "There is no started region with the name " << str_reg_name << std::endl;
            return;
        }

        cpu_id_t cpu_id = get_current_cpu();

        typename measure_data_t::thread_measure_data_t& thread_datum = datum->second.thread_data[cpu_id];

        if (thread_datum.started_time.tv_sec == 0 && thread_datum.started_time.tv_sec == 0) {
            std::cout << "Region " << str_reg_name << " not started\n";
            return;
        }

        struct timespec finish, runtime;
        clock_gettime(CLOCK_MONOTONIC, &finish);

        timespec_sub(&runtime, &finish, &thread_datum.started_time);
        timespec_add(&thread_datum.total_time, &thread_datum.total_time, &runtime);

        thread_datum.total_count++;
        thread_datum.started_time.tv_sec = 0;
        thread_datum.started_time.tv_nsec = 0;

        
    }

    static likwid_stub& get_instance() {
        static likwid_stub my_likwid;
        return my_likwid;
    }

private:
    using cpu_id_t = int;
    using cpu_affinity_mask_t = cpu_set_t;

    struct measure_data_t {
        struct thread_measure_data_t {
            int total_count;
            struct timespec total_time;
            struct timespec started_time;

            thread_measure_data_t() : total_count(0), total_time{0,0}, started_time{0, 0} {}
        };
    	int created;
        std::unordered_map<cpu_id_t, thread_measure_data_t> thread_data;

        measure_data_t() : created(-1) {}

	bool operator()(const measure_data_t& lhs, const measure_data_t& rhs) const { lhs.created < rhs.created; } 
    };


    static void timespec_add (struct timespec *sum, const struct timespec *left,
                const struct timespec *right)
    {
        sum->tv_sec = left->tv_sec + right->tv_sec;
        sum->tv_nsec = left->tv_nsec + right->tv_nsec;
        if (sum->tv_nsec >= 1000000000) {
            ++sum->tv_sec;
            sum->tv_nsec -= 1000000000;
        }
    }

    static void timespec_sub (struct timespec *diff, const struct timespec *left,
                const struct timespec *right) {
        diff->tv_sec = left->tv_sec - right->tv_sec;
        diff->tv_nsec = left->tv_nsec - right->tv_nsec;
        if (diff->tv_nsec < 0) {
            --diff->tv_sec;
            diff->tv_nsec += 1000000000;
        }
    }

    static double timespec_tosec(const struct timespec* diff) {
        double seconds;

        seconds = diff->tv_sec + (diff->tv_nsec / 1000000000.0);
        return seconds;
    }

    static cpu_id_t get_current_cpu() {
        return omp_get_thread_num();
    }


    std::unordered_map<std::string, measure_data_t> m_measure_data;
    int m_created_count;
};

#define LIKWID_MARKER_INIT (likwid_stub<int>::get_instance().init())
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_CLOSE (likwid_stub<int>::get_instance().close())
#define LIKWID_MARKER_START(c) (likwid_stub<int>::get_instance().start(c))
#define LIKWID_MARKER_STOP(c) (likwid_stub<int>::get_instance().stop(c))
