// ---------------------------------------------------------------------------
// File        : pinned_thread.h
// Project     : Beacon
// Component   : Common
// Description : Minimal HFT-style pinned thread wrapper (cross-platform)
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <thread>
#include <iostream>
#include <functional>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#elif defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <pthread.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
#endif

#include "cpu_pause.h" ///< defines _mm_pause() stub for ARM/Apple Silicon

class PinnedThread
{
public:
    PinnedThread() = default;

    PinnedThread(const PinnedThread &) = delete;
    PinnedThread &operator=(const PinnedThread &) = delete;
    PinnedThread(PinnedThread &&) noexcept = default;
    PinnedThread &operator=(PinnedThread &&) noexcept = default;

    /**
     * @brief Construct and start a thread pinned to a CPU core
     * @param fn User callable: receives std::atomic<bool>& stop
     * @param core CPU core index (ignored on macOS, optional on Windows)
     * @param stopFlag Atomic stop flag to signal shutdown
     */
    template <typename Fn>
    PinnedThread(Fn &&fn, int core, std::atomic<bool> &stopFlag)
    {
        _thread = std::thread([this, fn = std::forward<Fn>(fn), core, &stopFlag]() mutable
        {
            setAffinity(core);
            try {
                fn(stopFlag);
            } catch (const std::exception &e) {
                std::cerr << "[PinnedThread] Exception: " << e.what() << "\n";
            } catch (...) {
                std::cerr << "[PinnedThread] Unknown exception\n";
            }
        });
    }

    ~PinnedThread()
    {
        if (_thread.joinable())
            _thread.join();
    }

    void join()
    {
        if (_thread.joinable())
            _thread.join();
    }

    bool joinable() const { return _thread.joinable(); }

private:
    std::thread _thread;

    void setAffinity(int core)
    {
#if defined(__linux__)
        if(core >= 0) {
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(core, &cpuset);
            if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0)
                std::cerr << "[PinnedThread] Warning: Failed to set thread affinity on Linux\n";
        }
#elif defined(_WIN32)
        if(core >= 0) {
            DWORD mask = 1 << core;
            HANDLE handle = _thread.native_handle();
            if(SetThreadAffinityMask(handle, mask) == 0)
                std::cerr << "[PinnedThread] Warning: Failed to set thread affinity on Windows\n";
        }
#elif defined(__APPLE__)
        // macOS cannot pin to a specific core; use QoS class instead
        pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE, 0);
        std::cerr << "[PinnedThread] macOS: core pinning not supported; QoS class set\n";
#endif
    }
};
