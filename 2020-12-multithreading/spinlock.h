#include <atomic>
#include <thread>

struct spinlock {
    std::atomic<bool> lock_ = {0};

    void lock() noexcept {
        static constexpr int YIELD_TIMEOUT = 500;
        for (;;) {
            int count = YIELD_TIMEOUT;
            // Optimistically assume the lock is free on the first try
            if (!lock_.exchange(true, std::memory_order_acquire)) {
                return;
            }
            // Wait for lock to be released without generating cache misses
            while (lock_.load(std::memory_order_relaxed)) {
                // Issue X86 PAUSE or ARM YIELD instruction to reduce contention
                // between hyper-threads
                count--;
                if (count == 0) {
                    count = YIELD_TIMEOUT;
                    std::this_thread::yield();
                }
            }
        }
    }

    bool try_lock() noexcept {
        // First do a relaxed load to check if lock is free in order to prevent
        // unnecessary cache misses if someone does while(!try_lock())
        return !lock_.load(std::memory_order_relaxed) &&
               !lock_.exchange(true, std::memory_order_acquire);
    }

    void unlock() noexcept { lock_.store(false, std::memory_order_release); }
};
