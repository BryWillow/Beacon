#pragma once
#include <thread>
#include <pthread.h>

namespace beacon::hft::concurrency {

class ThreadUtils {
public:
    /** @brief Indicates that no CPU pinning was requested for a thread */
    static constexpr int NO_CPU_PINNING = -1;

    /**
     * @brief Pin a thread to a specific CPU core.
     *        On macOS this is a no-op, as explicit pinning isn't supported.
     */
    static void pinThreadToCore(std::thread& t, int core) {
#if defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);
        pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        (void)t;
        (void)core;
        // macOS does not expose CPU affinity APIs for user threads
#endif
    }
};

} // namespace beacon::hft::concurrency
