#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>

class function_runner {
public:
    template <typename FUNCTION>
    void run_async(FUNCTION f) {
        m_time_started = std::chrono::steady_clock::now();
        m_num_executed = 0;
        m_run_function = true;

        m_runner_thread = std::thread(run_function<FUNCTION>, f, this);
    }

    void stop() {
        m_run_function = false;
        m_runner_thread.join();

        m_num_executed = 0;
    }

    void reset_execution_rate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_time_started = std::chrono::steady_clock::now();
        m_num_executed = 0;
    }

    double get_execution_rate() {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto current_time = std::chrono::steady_clock::now();

        std::chrono::duration<double> diff = current_time - m_time_started;
        return static_cast<double>(m_num_executed) / diff.count();
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> m_time_started;
    uint64_t m_num_executed;
    std::mutex m_mutex;
    std::atomic<bool> m_run_function;

    std::thread m_runner_thread;

    template <typename FUNCTION>
    static void run_function(FUNCTION f, function_runner* fr) {
        while (fr->m_run_function) {
            f();

            fr->m_mutex.lock();
            fr->m_num_executed++;
            fr->m_mutex.unlock();
        }
    }
};
