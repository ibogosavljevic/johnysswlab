#pragma once

// Use on systems where LIKWID is not available
#include <unordered_map>
#include <map>
#include <string>
#include <time.h>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <mutex>
#include "omp.h"

#ifdef _WIN32
#ifndef NOMINMAX
# define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/resource.h>
#endif

#define LIKWID_COLLECT_ALL

#ifdef LIKWID_COLLECT_ALL
bool likwid_collect_all() { return true; }
#else
bool likwid_collect_all() { return false; }
#endif

template <typename UNUSED>
class likwid_stub {
public:
    void init() { m_created_count = 0; }
    void close() {
        struct compare_obj {
            bool operator() (const measure_data_t& lhs, const measure_data_t& rhs) const { return lhs.created < rhs.created; }
	    };

	    m_data_mutex.lock();

        std::vector<std::pair<std::string, measure_data_t>> m_measure_data_sorted(m_measure_data.begin(), m_measure_data.end());
	    std::sort(m_measure_data_sorted.begin(), m_measure_data_sorted.end(), [](const auto& lhs, const auto& rhs) -> bool { return lhs.second.created < rhs.second.created; }); 

        for (std::pair<std::string, measure_data_t> d: m_measure_data_sorted) {
            std::string& region_name = d.first;

            int id = 0;
            std::cout << "Region " << region_name << "\n";
	        std::vector<double> second_runtime;
            for (const auto & m: d.second.thread_data) {
		        double sec = timespec_tosec(&m.second.time.total);
                std::cout << "\tCPU id = " << id << ", count = " << m.second.total_count << ", runtime " << sec << " s\n";
                if (likwid_collect_all()) {
                    double user_time_s = timeval_tosec(&m.second.user_time.total);
                    double sys_time_s = timeval_tosec(&m.second.system_time.total);
                    std::cout << "\t\tUser time = " << user_time_s << " s, system time = " << sys_time_s << " s\n";
                    std::cout << "\t\tMinor faults = " << m.second.minor_faults.total << ", major faults = " << m.second.major_faults.total << ", ";
                    std::cout << "context switches = " << m.second.context_switches.total << "\n";
                }
		        second_runtime.push_back(sec);
                id++;
            }

	        double min_runtime = second_runtime[0];
	        double max_runtime = second_runtime[0];
	        double total_runtime = 0.0;
	        for (int i = 0; i < second_runtime.size(); i++) {
                double v = second_runtime[i];
                if (v < min_runtime) { min_runtime = v; }
                if (v > max_runtime) { max_runtime = v; }
                total_runtime += v;
	        }
	        std::cout << "\tSTAT, cummulative runtime = " << total_runtime << " s, min = " << min_runtime << " s, ";
	        std::cout << "avg = " << total_runtime / second_runtime.size() << " s, max = " << max_runtime << " s" << std::endl;
        }

	    m_data_mutex.unlock();

        std::cout.flush();
    }

    void start(const char * region_name) {
        std::string str_reg_name(region_name);
        m_data_mutex.lock();

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

        if (thread_datum.time.started.tv_sec != 0 || thread_datum.time.started.tv_nsec != 0) {
            std::cout << "Region " << str_reg_name << " already started. Restarting\n";
        }

        clock_gettime_monotonic(&thread_datum.time.started);
        /*if (constexpr likwid_collect_all()) {
            struct rusage usage;
            if (getrusage(RUSAGE_THREAD, &usage) == 0) {
                thread_datum.user_time.started = usage.ru_utime;
                thread_datum.system_time.started = usage.ru_stime;
                thread_datum.minor_faults.started = usage.ru_minflt;
                thread_datum.major_faults.started = usage.ru_majflt;
                thread_datum.context_switches.started = usage.ru_nivcsw + usage.ru_nvcsw;
            }
        }*/

	    m_data_mutex.unlock();
    }

    void stop(const char * region_name) {
        std::string str_reg_name(region_name);

	    m_data_mutex.lock();

        auto datum = m_measure_data.find(str_reg_name);

        if (datum == m_measure_data.end()) {
            std::cout << "There is no started region with the name " << str_reg_name << std::endl;
            return;
        }

        cpu_id_t cpu_id = get_current_cpu();

        typename measure_data_t::thread_measure_data_t& thread_datum = datum->second.thread_data[cpu_id];

        if (thread_datum.time.started.tv_sec == 0 && thread_datum.time.started.tv_sec == 0) {
            std::cout << "Region " << str_reg_name << " not started\n";
            return;
        }

        struct timespec finish, runtime;
        clock_gettime_monotonic(&finish);

        timespec_sub(&runtime, &finish, &thread_datum.time.started);
        timespec_add(&thread_datum.time.total, &thread_datum.time.total, &runtime);

        thread_datum.total_count++;
        thread_datum.time.started.tv_sec = 0;
        thread_datum.time.started.tv_nsec = 0;

        /*if (constexpr likwid_collect_all()) {
            struct rusage usage;
            if (getrusage(RUSAGE_THREAD, &usage) == 0) {
                struct timeval runtime_user, runtime_system;
                timeval_sub(&runtime_user, &usage.ru_utime, &thread_datum.user_time.started);
                timeval_sub(&runtime_system, &usage.ru_stime, &thread_datum.system_time.started);
                
                timeval_add(&thread_datum.user_time.total, &thread_datum.user_time.total, &runtime_user);
                timeval_add(&thread_datum.system_time.total, &thread_datum.system_time.total, &runtime_system);
                
                thread_datum.minor_faults.total += usage.ru_minflt - thread_datum.minor_faults.started;
                thread_datum.major_faults.total += usage.ru_majflt - thread_datum.major_faults.started;
                thread_datum.context_switches.total += (usage.ru_nivcsw + usage.ru_nvcsw) - thread_datum.context_switches.started;
            }
        }*/

        m_data_mutex.unlock();
    }

    static likwid_stub& get_instance() {
        static likwid_stub my_likwid;
        return my_likwid;
    }

private:
    using cpu_id_t = int;
    //using cpu_affinity_mask_t = cpu_set_t;

    template <typename T>
    struct measurement_t {
        T total;
        T started;

        measurement_t(T val) : total(val), started(val) {}
    };

    struct measure_data_t {
        struct thread_measure_data_t {
            int total_count;
            measurement_t<struct timespec> time;
            measurement_t<struct timeval> user_time;
            measurement_t<struct timeval> system_time;
            measurement_t<long> minor_faults;
            measurement_t<long> major_faults;
            measurement_t<long> context_switches;

            thread_measure_data_t() : 
                total_count(0), time({0,0}),
                user_time({0, 0}), system_time({0, 0}),
                minor_faults(0), major_faults(0),
                context_switches(0)
            {}
        };
    	int created;
        std::unordered_map<cpu_id_t, thread_measure_data_t> thread_data;

        measure_data_t() : created(-1) {}

	bool operator()(const measure_data_t& lhs, const measure_data_t& rhs) const { return lhs.created < rhs.created; } 
    };

#ifdef _WIN32
    struct timespec { long tv_sec; long tv_nsec; };
    struct timeval { long tv_sec; long tv_usec; };

    int clock_gettime_monotonic(struct timespec *tv) {
        __int64 wintime; GetSystemTimeAsFileTime((FILETIME*)&wintime);
        wintime      -=116444736000000000i64;  //1jan1601 to 1jan1970
        tv->tv_sec  =wintime / 10000000i64;           //seconds
        tv->tv_nsec =wintime % 10000000i64 *100;      //nano-seconds
        return 0;
    }
#else
    int clock_gettime_monotonic(struct timespec *tv) {
        return clock_gettime(CLOCK_MONOTONIC, tv);
    }
#endif

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


    static void timeval_add (struct timeval *sum, const struct timeval *left,
                const struct timeval *right)
    {
        sum->tv_sec = left->tv_sec + right->tv_sec;
        sum->tv_usec = left->tv_usec + right->tv_usec;
        if (sum->tv_usec >= 1000000) {
            ++sum->tv_sec;
            sum->tv_usec -= 1000000;
        }
    }

    static void timeval_sub (struct timeval *diff, const struct timeval *left,
                const struct timeval *right) {
        diff->tv_sec = left->tv_sec - right->tv_sec;
        diff->tv_usec = left->tv_usec - right->tv_usec;
        if (diff->tv_usec < 0) {
            --diff->tv_sec;
            diff->tv_usec += 1000000;
        }
    }

    static double timeval_tosec(const struct timeval* diff) {
        double seconds;

        seconds = diff->tv_sec + (diff->tv_usec / 1000000.0);
        return seconds;
    }

    static cpu_id_t get_current_cpu() {
        return 0;
	    //return omp_get_thread_num();
    }


    std::unordered_map<std::string, measure_data_t> m_measure_data;
    std::mutex m_data_mutex;
    int m_created_count;
};

#define LIKWID_MARKER_INIT (likwid_stub<int>::get_instance().init())
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_CLOSE (likwid_stub<int>::get_instance().close())
#define LIKWID_MARKER_START(c) (likwid_stub<int>::get_instance().start(c))
#define LIKWID_MARKER_STOP(c) (likwid_stub<int>::get_instance().stop(c))
