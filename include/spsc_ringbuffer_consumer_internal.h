// ---------------------------------------------------------------------------
// File        : spsc_ringbuffer_consumer_internal.h
// Project     : HftSimulator
// Component   : Common
// Description : Internal consumer for single-producer single-consumer ring buffer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>

#include "utils/cpu_pause.h"
#include "spsc_ringbuffer.h"
#include "nsdq/md_itch_message.h"

/**
 * @class RingBufferConsumerInternal
 * @brief Internal class that consumes messages from a single-producer, single-consumer ring buffer.
 *
 * Users should use the factory function make_ring_buffer_consumer() instead of instantiating this directly.
 *
 * Template Parameters:
 * - MsgType: type stored in the ring buffer (e.g., ItchMessage)
 * - Callback: callable invoked for each popped message
 * - N: compile-time capacity of the ring buffer
 *
 * Hot-path notes:
 * - Only the consumer modifies `_tail`; only the producer modifies `_head`.
 * - Relaxed atomic counters for stats; memory_order_relaxed is sufficient.
 * - `_mm_pause()` reduces CPU pressure while spinning when the buffer is empty.
 * - Thread can be pinned to a CPU core for deterministic cache locality.
 */
template <typename MsgType, typename Callback, size_t N>
class RingBufferConsumerInternal {
public:
    /**
     * @brief Construct the consumer with a reference to the buffer and a callback.
     * @param buffer Reference to the SPSC buffer
     * @param callback Callable invoked for each popped message
     *
     * Tricky parts:
     * - `Callback` is templated to allow inlining of lambdas and avoid virtual dispatch.
     * - Using std::move on the callback ensures we don’t copy large state unnecessarily.
     */
    RingBufferConsumerInternal(SpScRingBuffer<MsgType, N>& buffer, Callback callback)
        : _buffer(buffer), _callback(std::move(callback)), _stopFlag(false) {}

    /**
     * @brief Start the consumer thread, optionally pinned to a CPU core.
     * @param core CPU core index to pin thread to (-1 for no pinning)
     *
     * Tricky parts:
     * - Pinning improves cache locality and determinism in HFT environments.
     * - Thread lambda captures `this` safely; hot-path remains fully inlined.
     */
    void start(size_t core = -1) {
        _thread = std::thread([this, core] { consumeLoop(core); });
        if (core >= 0) pinThreadToCore(_thread, core);
    }

    /**
     * @brief Stop the consumer thread gracefully.
     *
     * Tricky parts:
     * - `_stopFlag` uses relaxed atomic ordering; thread observes it in spin loop.
     * - Join ensures the thread has finished before destruction.
     */
    void stop() noexcept {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread.joinable()) _thread.join();
    }

private:
    SpScRingBuffer<MsgType, N>& _buffer;    ///< Hot-path buffer reference
    Callback                    _callback;  ///< Hot-path callback
    std::atomic<bool>           _stopFlag;  ///< Relaxed atomic stop flag
    std::thread                 _thread;    ///< Consumer thread

    /**
     * @brief Core consuming loop
     *
     * Tricky parts:
     * - Uses a stack-allocated `msg{}` for each pop; avoids dynamic allocation.
     * - `_buffer.tryPop(msg)` is highly inlined and uses cache-line-aligned head/tail.
     * - `_mm_pause()` reduces aggressive CPU spinning without adding latency.
     * - Callback is called immediately after pop; fully inlined if lambda.
     */
    void consumeLoop(size_t /*core*/) {
        MsgType msg{};
        while (!_stopFlag.load(std::memory_order_relaxed)) {
            if (_buffer.tryPop(msg)) {
                _callback(msg);
            } else {
                _mm_pause(); // reduces busy-wait pressure
            }
        }
    }

    /**
     * @brief Pin a thread to a specific CPU core.
     *
     * Tricky parts:
     * - Linux implementation uses pthread_setaffinity_np.
     * - macOS stub included; pinning omitted as no official API exists in standard C++.
     */
    static void pinThreadToCore(std::thread& t, size_t core) {
#if defined(__linux__)
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);
        pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
#elif defined(__APPLE__)
        (void)t;
        (void)core;
#endif
    }
};

/**
 * @brief Factory function to create a RingBufferConsumer without exposing template parameters
 *
 * @tparam MsgType type stored in the ring buffer
 * @tparam N       compile-time buffer size
 * @tparam Callback type of callable for processing messages
 * @param buffer reference to the ring buffer
 * @param callback callable invoked for each popped message
 * @return fully constructed RingBufferConsumerInternal
 *
 * Tricky parts:
 * - Callback type is deduced via perfect forwarding; allows inlining of lambdas.
 * - Users do not see the internal template parameters; clean API for HFT usage.
 */
template <typename MsgType, size_t N, typename Callback>
auto make_ring_buffer_consumer(SpScRingBuffer<MsgType, N>& buffer, Callback&& callback) {
    return RingBufferConsumerInternal<MsgType, std::decay_t<Callback>, N>(
        buffer, std::forward<Callback>(callback)
    );
}