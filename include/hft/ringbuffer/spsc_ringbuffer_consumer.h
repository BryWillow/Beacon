// ---------------------------------------------------------------------------
// @file        spsc_ringbuffer_consumer.h
// Project     : Beacon
// Component   : HFT RingBuffer
// Description : Header-only single-producer single-consumer ring buffer consumer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>

#include "spsc_ringbuffer.h"
#include "utils/cpu_pause.h"
#include "core/constants.h"

namespace beacon::hft::ringbuffer
{

    /**
     * @class SpScRingBufferConsumer
     * @brief Threaded consumer for a single-producer single-consumer ring buffer.
     *
     * Hot-path optimizations:
     * - Only the consumer modifies `_tail`; only the producer modifies `_head`.
     * - `_mm_pause()` reduces CPU pressure while spinning.
     * - Relaxed atomics for stop flag; memory_order_relaxed is sufficient.
     * - Optional CPU pinning for cache locality.
     *
     * @tparam MsgType Type stored in the ring buffer (e.g., ItchMessage)
     * @tparam Callback Callable invoked for each popped message
     * @tparam N Compile-time capacity of the ring buffer
     */
    template <typename MsgType, typename Callback, size_t N = beacon::constants::DEFAULT_RING_BUFFER_CAPACITY>
    class SpScRingBufferConsumer
    {
    public:
        /**
         * @brief Construct the consumer with a reference to the buffer and a callback.
         * @param buffer Reference to the SPSC buffer
         * @param callback Callable invoked for each popped message
         */
        SpScRingBufferConsumer(SpScRingBuffer<MsgType, N> &buffer, Callback callback)
            : _buffer(buffer), _callback(std::move(callback)), _stopFlag(false) {}

        /**
         * @brief Start the consumer thread, optionally pinned to a CPU core.
         * @param core CPU core index to pin thread to (-1 for no pinning)
         */
        void start(int core = beacon::constants::NO_CPU_PINNING)
        {
            _thread = std::thread([this]
                                  { consumeLoop(); });
            if (core >= 0)
                pinThreadToCore(_thread, core);
        }

        /**
         * @brief Stop the consumer thread gracefully.
         */
        void stop() noexcept
        {
            _stopFlag.store(true, std::memory_order_relaxed);
            if (_thread.joinable())
                _thread.join();
        }

    private:
        SpScRingBuffer<MsgType, N> &_buffer; ///< Reference to SPSC buffer
        Callback _callback;                  ///< Hot-path callback
        std::atomic<bool> _stopFlag;         ///< Relaxed atomic stop flag
        std::thread _thread;                 ///< Consumer thread

        void consumeLoop()
        {
            MsgType msg{};
            while (!_stopFlag.load(std::memory_order_relaxed))
            {
                if (_buffer.tryPop(msg))
                {
                    _callback(msg);
                }
                else
                {
                    _mm_pause(); // reduces busy-wait pressure
                }
            }
        }

        static void pinThreadToCore(std::thread &t, int core)
        {
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
     * @brief Factory function for SpScRingBufferConsumer to hide template parameters.
     *
     * @tparam MsgType Type stored in the ring buffer
     * @tparam N Compile-time capacity
     * @tparam Callback Callable type
     * @param buffer Reference to ring buffer
     * @param callback Callable invoked for each popped message
     * @return Fully constructed SpScRingBufferConsumer
     */
    template <typename MsgType, size_t N, typename Callback>
    auto make_sp_sc_ringbuffer_consumer(SpScRingBuffer<MsgType, N> &buffer, Callback &&callback)
    {
        return SpScRingBufferConsumer<MsgType, std::decay_t<Callback>, N>(
            buffer, std::forward<Callback>(callback));
    }

} // namespace beacon::hft::ringbuffer
