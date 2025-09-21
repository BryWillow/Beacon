// ---------------------------------------------------------------------------
// File        : spsc_ringbuffer_consumer.h
// Project     : Beacon
// Component   : Common
// Description : Consumer for single-producer single-consumer ring buffer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <stdexcept>
#include <format>        // C++20 std::format for error messages

#include "spsc_ringbuffer.h"
#include "cpu_pause.h"

/**
 * @brief Special value indicating no CPU pinning requested
 */
inline constexpr int NO_PINNING = -1;

/**
 * @class SpScRingBufferConsumer
 * @brief Consumes messages from a single-producer, single-consumer ring buffer in a thread.
 *
 * Template Parameters:
 * - MsgType : type stored in the ring buffer (e.g., ItchMessage)
 * - Callback: callable invoked for each popped message
 *
 * Hot-path considerations:
 * - Consumer only modifies _tail; producer only modifies _head.
 * - Relaxed atomics ensure low-latency visibility.
 * - _mm_pause() reduces CPU pressure while spinning.
 * - Thread is optionally pinned to a CPU core for cache locality.
 */
template <typename MsgType, typename Callback>
class SpScRingBufferConsumer {
public:
    /**
     * @brief Construct with reference to buffer and a callback
     * @param buffer Reference to SPSC ring buffer
     * @param callback Callable invoked for each popped message
     *
     * Notes:
     * - Callback is moved to avoid unnecessary copies.
     * - Only one consumer per buffer allowed.
     */
    SpScRingBufferConsumer(SpScRingBuffer<MsgType>& buffer, Callback callback)
        : _buffer(buffer), _callback(std::move(callback)), _stopFlag(false) {}

    /**
     * @brief Start consuming messages on a thread (optionally pinned)
     * @param core CPU core index to pin thread (-1 = no pinning)
     */
    void start(int core = NO_PINNING) {
        // Instantiate and start the consumer thread
        _thread = std::thread([this, core] { consumeLoop(core); });

        // Pin the thread if requested
        if (core != NO_PINNING) pinThreadToCore(_thread, core);
    }

    /**
     * @brief Stop consumer gracefully
     * - Sets atomic stop flag and joins the thread
     */
    void stop() noexcept {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread.joinable()) _thread.join();
    }

private:
    SpScRingBuffer<MsgType>& _buffer;  ///< Reference to ring buffer
    Callback _callback;                ///< Hot-path callback for each popped message
    std::atomic<bool> _stopFlag;       ///< Relaxed atomic stop flag
    std::thread _thread;               ///< Consumer thread

    /**
     * @brief Core consuming loop
     * @param core CPU core number (used for pinning info)
     *
     * Notes:
     * - Uses stack-allocated MsgType to avoid dynamic allocations.
     * - Callback is invoked immediately after successful pop.
     * - Fully inlined if lambda is provided.
     */
    void consumeLoop(int /*core*/) {
        MsgType msg{};
        while (!_stopFlag.load(std::memory_order_relaxed)) {
            if (_buffer.tryPop(msg)) {
                _callback(msg);
            } else {
                _mm_pause(); // reduce CPU spinning
            }
        }
    }

    /**
     * @brief Pin thread to CPU core
     * @param t Thread to pin
     * @param core Core index
     * @throws std::runtime_error if core index invalid (Linux)
     *
     * Notes:
     * - Linux implementation uses pthread_setaffinity_np.
     * - macOS does not support pinning; stub included for cross-platform compilation.
     */
    static void pinThreadToCore(std::thread& t, int core) {
#if defined(__linux__)
        int ncores = std::thread::hardware_concurrency();
        if (core < 0 || static_cast<unsigned>(core) >= ncores) {
            throw std::runtime_error(std::format(
                "Invalid CPU core {} for pinning; max cores {}", core, ncores));
        }
        cpu_set_t cpuset{};
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);
        pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        (void)t;    // intentionally unused
        (void)core; // intentionally unused
#endif
    }
};
