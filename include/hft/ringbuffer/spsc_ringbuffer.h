// ---------------------------------------------------------------------------
// @file        spsc_ringbuffer.h
// Project     : Beacon
// Component   : HFT Core
// Description : Cache-line optimized single-producer single-consumer ring buffer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <cstddef>
#include <array>
#include <algorithm>
#include "core/constants.h" // for DEFAULT_RING_BUFFER_CAPACITY

namespace beacon::hft::ringbuffer
{

    /**
     * @class SpScRingBuffer
     * @brief Lock-free, fixed-capacity ring buffer for single producer / single consumer.
     *
     * Template Parameters:
     * - T: type stored in the buffer
     * - Capacity: compile-time capacity (defaults to core constants)
     *
     * Notes:
     * - Only the producer thread modifies `_head`; only the consumer thread modifies `_tail`.
     * - Supports relaxed atomic operations for high-performance HFT scenarios.
     * - Tracks dropped messages when buffer is full.
     * - High-water mark optionally monitored.
     */
    template <typename T, size_t Capacity = beacon::hft::core::DEFAULT_RING_BUFFER_CAPACITY>
    class SpScRingBuffer
{
public:
        SpScRingBuffer()
            : _head(0), _tail(0), _dropped(0), _highWaterMark(0) {}

        SpScRingBuffer(const SpScRingBuffer &) = delete;
        SpScRingBuffer &operator=(const SpScRingBuffer &) = delete;

        SpScRingBuffer(SpScRingBuffer &&) = delete;
        SpScRingBuffer &operator=(SpScRingBuffer &&) = delete;

        /**
         * @brief Attempt to push an item into the buffer.
         * @param item The item to push
         * @return true if successfully pushed, false if buffer was full
         */
        bool tryPush(const T &item)
        {
            const size_t head = _head.load(std::memory_order_relaxed);
            const size_t next = increment(head);

            if (next == _tail.load(std::memory_order_acquire))
            {
                _dropped.fetch_add(1, std::memory_order_relaxed);
                return false;
            }

            _buffer[head] = item;
            _head.store(next, std::memory_order_release);
            updateHighWaterMark(next);
            return true;
        }

        /**
         * @brief Attempt to pop an item from the buffer.
         * @param item Output parameter to store the popped item
         * @return true if successfully popped, false if buffer was empty
         */
        bool tryPop(T &item)
        {
            const size_t tail = _tail.load(std::memory_order_relaxed);
            if (tail == _head.load(std::memory_order_acquire))
            {
                return false;
            }

            item = _buffer[tail];
            _tail.store(increment(tail), std::memory_order_release);
            return true;
        }

        /**
         * @brief Returns number of messages dropped due to full buffer
         */
        size_t dropped() const { return _dropped.load(std::memory_order_relaxed); }

        /**
         * @brief Returns the current high-water mark
         */
        size_t highWaterMark() const { return _highWaterMark.load(std::memory_order_relaxed); }

    private:
        std::array<T, Capacity> _buffer;
        alignas(64) std::atomic<size_t> _head;
        alignas(64) std::atomic<size_t> _tail;
        std::atomic<size_t> _dropped;
        std::atomic<size_t> _highWaterMark;

        size_t increment(size_t idx) const noexcept { return (idx + 1) % Capacity; }

        void updateHighWaterMark(size_t head)
        {
            size_t current = _highWaterMark.load(std::memory_order_relaxed);
            size_t used = (head + Capacity - _tail.load(std::memory_order_relaxed)) % Capacity;
            while (used > current && !_highWaterMark.compare_exchange_weak(current, used, std::memory_order_relaxed))
            {
            }
        }
    };

} // namespace beacon::hft::ringbuffer
