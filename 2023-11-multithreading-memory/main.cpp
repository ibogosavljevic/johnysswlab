#include <cstdint>
#include <cstddef>
#include <memory>
#include <array>
#include <mutex>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

#include <emmintrin.h>

template<typename T, size_t SIZE=64>
class thread_safe_queue_t {
public:
    thread_safe_queue_t():
        front(0),
        back(0)
    { }

    bool push(T v) {
        std::unique_lock<std::mutex> lock(m_mutex); 

        size_t new_front = front + 1;
        if (new_front == SIZE) {
            new_front = 0;
        }

        // List empty
        if (new_front == back) {
            return false;
        }

        m_array[new_front] = v;
        front = new_front;
        return true;
    }

    bool pop(T& v) {
        std::unique_lock<std::mutex> lock(m_mutex); 

        if (front == back) {
            return false;
        }

        size_t new_back = back + 1;
        if (new_back == SIZE) {
            new_back = 0;
        }

        v = m_array[back];
        back = new_back;

        return true;
    }

private:
    std::mutex m_mutex;
    std::array<T, SIZE> m_array;
    size_t front;
    size_t back;

};

struct data_packet_t{
    static constexpr size_t PACKET_SIZE = 128;
    int32_t keys[PACKET_SIZE];
    int32_t idx[PACKET_SIZE];
};

using packet_queue_t = thread_safe_queue_t<data_packet_t>;

struct packet_dispatcher_t {
    struct packet_processor_t {
        packet_queue_t q;
        int32_t to_val;

        data_packet_t packet;
        size_t packet_size;

        packet_processor_t() : 
          packet_size(0)
        { }
    };

    std::vector<packet_processor_t> queues;
    size_t current_queue;
    size_t total_queues;

    packet_dispatcher_t(size_t num_queues, int32_t* sorted, size_t sorted_size) : 
        queues(num_queues),
        current_queue(0),
        total_queues(num_queues)
    {
        for (size_t i = 0; i < num_queues; i++) {
            queues[i].to_val = sorted[sorted_size / num_queues * (i + 1)];
        }
        queues[num_queues - 1].to_val = std::numeric_limits<int32_t>::max();
    }

    // Enques a value until a packet is formed, then dispatch the packet
    void enqueue(int32_t val) {
        size_t queues_size = queues.size();
        for (size_t i = 0; i < queues_size; i++) {
            if (val <= queues[i].to_val) {
                queues[i].packet.keys[queues[i].packet_size] = val;
                queues[i].packet_size++;

                if (queues[i].packet_size == data_packet_t::PACKET_SIZE) {
                    bool res = queues[i].q.push(queues[i].packet);
                    if (res == false) {
                        //std::cout << "Packet dropped\n";
                    } else {
                        //std::cout << "Packet pushed\n";
                    }
                    queues[i].packet_size = 0;
                }

                return;
            }
        }
    }

    // Enques a value until a packet is formed, then dispatch the packet
    void enqueue_round_robin(int32_t val) {
        queues[current_queue].packet.keys[queues[current_queue].packet_size] = val;
        queues[current_queue].packet_size++;

        if (queues[current_queue].packet_size == data_packet_t::PACKET_SIZE) {
            bool res = queues[current_queue].q.push(queues[current_queue].packet);
            if (res == false) {
                //std::cout << "Packet dropped\n";
            }
            queues[current_queue].packet_size = 0;
        }

        current_queue++;
        if (current_queue == total_queues) {
            current_queue = 0;
        }
    }
};

template <bool assign_round_robin = true>
void generate_data(packet_dispatcher_t* dispatcher, int32_t min, int32_t max, bool* finish) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int32_t>  distr(min, max);

    if (assign_round_robin) {
        while (!*finish) {
            dispatcher->enqueue_round_robin(distr(generator));
        }
    } else {
        while (!*finish) {
            dispatcher->enqueue(distr(generator));
        }
    }
}


int32_t binary_search(int32_t* sorted, size_t sorted_size, int32_t key) {
    int low = 0;
    int high = sorted_size - 1;
    int mid;

    while(low <= high) {
        mid = (low + high) / 2;

        if (sorted[mid] < key) {
            low = mid + 1; 
        } else if(sorted[mid] > key) {
            high = mid - 1;
        } else {
            return mid;
            break;
        }
    }

    return std::numeric_limits<int32_t>::min();
}

struct binary_search_params_t {
    int32_t* sorted;
    size_t sorted_size;
    packet_queue_t* my_queue;

    bool* finish;
    std::pair<size_t, uint64_t>* res;
};

void binary_search_packet(binary_search_params_t params) {
    int32_t* sorted = params.sorted;
    size_t sorted_size = params.sorted_size;
    packet_queue_t* my_queue = params.my_queue;
    bool* finish = params.finish;
    std::pair<size_t, uint64_t>* res = params.res;

    data_packet_t packet;
    size_t total_packets = 0;

    uint64_t time = 0;

    while(!*finish) {
        bool res = my_queue->pop(packet);
        if (res) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (size_t i = 0; i < packet.PACKET_SIZE; ++i) {
                packet.idx[i] = binary_search(sorted, sorted_size, packet.keys[i]);
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            time += std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count();
            total_packets++;
        } else {
            _mm_pause();
        }
    }

    *res = std::make_pair(total_packets, time);
}

void generate_sorted_array(int32_t* array, size_t size) {
    int32_t gen = 0;
    for (size_t i = 0; i < size; ++i) {
        array[i] = gen;
        gen += rand() % 3 + 1;
    }
}

template<bool use_round_robin>
void run_test(size_t sorted_size, size_t num_threads) {
    bool finish = false;

    std::vector<int32_t> sorted_from(sorted_size);
    generate_sorted_array(sorted_from.data(), sorted_size);

    int32_t* sorted = (int32_t*) malloc(sizeof(int32_t) * sorted_size);

    packet_dispatcher_t packet_dispatcher(num_threads, sorted_from.data(), sorted_size);

    std::thread generator_thread(generate_data<use_round_robin>, &packet_dispatcher, sorted_from[0] - 10, sorted_from[sorted_size - 1] + 10, &finish);
    std::vector<std::thread> search_threads;
    std::vector<std::pair<size_t, uint64_t>> search_results;
    search_results.resize(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        binary_search_params_t params;
        params.sorted_size = sorted_size;
        params.my_queue = &(packet_dispatcher.queues[i].q);
        params.sorted = sorted_from.data();
        params.finish = &finish;
        params.res = &search_results[i];

        search_threads.emplace_back(binary_search_packet, params);
    }

    std::cout << "SIZE = " << sorted_size << ", ROUND_ROBIN = " << use_round_robin << "\n";
    std::cout << "Threads started. Waiting 5 seconds\n";

    using namespace std::chrono_literals;

    std::this_thread::sleep_for(5s);

    finish = true;
    generator_thread.join();

    uint64_t all_threads_runtime = 0;
    size_t all_threads_packets = 0;

    for (size_t i = 0; i < num_threads; i++) {
        search_threads[i].join();
        all_threads_packets += search_results[i].first;
        all_threads_runtime += search_results[i].second;
        std::cout << "Thr " << i << ": total packets = " << search_results[i].first 
                  << ", average time per packet " << search_results[i].second / search_results[i].first << "\n";
    }

    std::cout << "For all threads: total packets = " << all_threads_packets 
              << ", average time per packet = " << all_threads_runtime / all_threads_packets << "\n";

    free(sorted);
}

int main() {
    static constexpr size_t start_size = 1024;
    static constexpr size_t end_size = 64*1024*1024;
    static constexpr size_t THREAD_COUNT = 8;

    for (size_t s = start_size; s <= end_size; s *= 4) {
        size_t size = s - 348;
        run_test<true>(size, THREAD_COUNT);
        run_test<false>(size, THREAD_COUNT);
    }

    return 0;
}