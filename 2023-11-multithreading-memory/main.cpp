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

void binary_search_packet(int32_t* sorted, size_t sorted_size, packet_queue_t* my_queue, bool* finish, std::pair<size_t, uint64_t>* res) {
    data_packet_t packet;
    size_t total_packets = 0;

    uint64_t diff = 0;

    while(!*finish) {
        bool res = my_queue->pop(packet);
        if (res) {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            for (size_t i = 0; i < packet.PACKET_SIZE; ++i) {
                packet.idx[i] = binary_search(sorted, sorted_size, packet.keys[i]);
            }
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            diff += std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count();
            total_packets++;
        } else {
            _mm_pause();
        }
    }

    *res = std::make_pair(total_packets, diff);
}

void generate_sorted_array(int32_t* array, size_t size) {
    int32_t gen = 0;
    for (size_t i = 0; i < size; ++i) {
        array[i] = gen;
        gen += rand() % 3 + 1;
    }
}

int main() {
    static constexpr size_t SORTED_SIZE = 128 * 1024 - 128;
    static constexpr size_t NUM_THREADS = 4;
    static constexpr bool USE_ROUND_ROBIN = true;

    bool finish = false;

    std::vector<int32_t> sorted(SORTED_SIZE);
    generate_sorted_array(sorted.data(), SORTED_SIZE);

    packet_dispatcher_t packet_dispatcher(NUM_THREADS, sorted.data(), SORTED_SIZE);

    std::thread generator_thread(generate_data<USE_ROUND_ROBIN>, &packet_dispatcher, sorted[0] - 10, sorted[SORTED_SIZE - 1] + 10, &finish);
    std::vector<std::thread> search_threads;
    std::vector<std::pair<size_t, uint64_t>> search_results;
    search_results.resize(NUM_THREADS);

    for (size_t i = 0; i < NUM_THREADS; ++i) {
        search_threads.emplace_back(binary_search_packet, sorted.data(), SORTED_SIZE, &(packet_dispatcher.queues[i].q), &finish, &search_results[i]);
    }

    std::cout << "Threads started. Waiting 5 seconds\n";

    using namespace std::chrono_literals;

    std::this_thread::sleep_for(5s);

    finish = true;
    generator_thread.join();

    uint64_t all_threads_runtime = 0;
    size_t all_threads_packets = 0;

    for (size_t i = 0; i < NUM_THREADS; i++) {
        search_threads[i].join();
        all_threads_packets += search_results[i].first;
        all_threads_runtime += search_results[i].second;
        std::cout << "Thr " << i << ": total packets = " << search_results[i].first 
                  << ", average time per packet " << search_results[i].second / search_results[i].first << "\n";
    }

    std::cout << "For all threads: total packets = " << all_threads_packets 
              << ", average time per packet = " << all_threads_runtime / all_threads_packets << "\n";

    return 0;
}