// Use on systems where LIKWID is not available
#include <unordered_map>
#include <map>
#include <string>
#include <time.h>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>

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

        for (auto d: m_measure_data_sorted) {
            std::cout << "Region: " << d.first << ", count " << d.second.total_count << ", total runtime " << timespec_tosec(&d.second.total_time) << std::endl;
        }
    }

    void start(const char * region_name) {
        std::string str_reg_name(region_name);
        measure_data_t* my_datum = &m_measure_data[str_reg_name];

	if (my_datum->created == -1) {
            my_datum->created = m_created_count;
	    m_created_count++;
	}

        if (my_datum->started_time.tv_sec != 0 || my_datum->started_time.tv_nsec != 0) {
            std::cout << "Region " << str_reg_name << " already started. Restarting\n";
        }

        clock_gettime(CLOCK_MONOTONIC, &my_datum->started_time);
    }

    void stop(const char * region_name) {
        std::string str_reg_name(region_name);
        auto datum = m_measure_data.find(str_reg_name);

        if (datum == m_measure_data.end()) {
            std::cout << "There is no started region with the name " << str_reg_name << std::endl;
            return;
        }

        if (datum->second.started_time.tv_sec == 0 && datum->second.started_time.tv_sec == 0) {
            std::cout << "Region " << str_reg_name << " not started\n";
            return;
        }

        struct timespec finish, runtime;
        clock_gettime(CLOCK_MONOTONIC, &finish);

        timespec_sub(&runtime, &finish, &datum->second.started_time);
        timespec_add(&datum->second.total_time, &datum->second.total_time, &runtime);

        datum->second.total_count++;
        datum->second.started_time.tv_sec = 0;
        datum->second.started_time.tv_nsec = 0;
    }

    static likwid_stub& get_instance() {
        static likwid_stub my_likwid;
        return my_likwid;
    }

private:
    struct measure_data_t {
        int total_count;
        struct timespec total_time;
        struct timespec started_time;
	int created;

        measure_data_t() : total_count(0), total_time{0,0}, started_time{0, 0}, created(-1) {}

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

    static double timespec_tosec(struct timespec* diff) {
        double seconds;

        seconds = diff->tv_sec + (diff->tv_nsec / 1000000000.0);
        return seconds;
    }


    std::unordered_map<std::string, measure_data_t> m_measure_data;
    int m_created_count;
};

#define LIKWID_MARKER_INIT (likwid_stub<int>::get_instance().init())
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_CLOSE (likwid_stub<int>::get_instance().close())
#define LIKWID_MARKER_START(c) (likwid_stub<int>::get_instance().start(c))
#define LIKWID_MARKER_STOP(c) (likwid_stub<int>::get_instance().stop(c))
