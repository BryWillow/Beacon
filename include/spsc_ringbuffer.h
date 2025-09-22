// ---------------------------------------------------------------------------
// File        : spsc_ringbuffer.h
// Project     : HftSimulator
// Component   : Common
// Description : Lock-free single-producer single-consumer ring buffer
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <atomic>
#include <type_traits>  // for noexcept checks on copy/move
#include <thread>
#include <iostream>

#include "constants.h"

#if defined(__x86_64__)
#include <immintrin.h>  // x86-only cache-line optimizations
#endif

/**
 * @class SpScRingBuffer
 * @brief Single-Producer Single-Consumer Ring Buffer (SPSC)
 *
 * Highly optimized for low-latency HFT pipelines:
 * - Single producer thread writes into the buffer.
 * - Single consumer thread reads from the buffer.
 * - Cache-line aligned head/tail and storage to prevent false sharing.
 * - Atomic counters track statistics; memory_order_relaxed is sufficient
 *   since only one producer/consumer updates each value.
 *
 * Works well with ItchUdpListener: listener pushes messages on a pinned thread,
 * while consumer thread reads messages immediately.
 *
 * @tparam T        Type of element stored (preferably POD or trivially copyable)
 * @tparam Capacity Compile-time capacity (must be power-of-2, >=2)
 */
template <typename T, size_t Capacity = hft::DEFAULT_RING_BUFFER_CAPACITY>
class SpScRingBuffer {
    // Compile-time validation: power-of-2 capacity
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power-of-2");
    static_assert(Capacity >= 2, "Capacity must be at least 2");

public:
    SpScRingBuffer() noexcept
        : _head{0}, _tail{0},
          _droppedMessageCount{0}, _pushedMessageCount{0}, _poppedMessageCount{0},
          _highWaterMark{0} {}

    // Deleted copy/move constructors: buffer is not copyable/movable
    SpScRingBuffer(const SpScRingBuffer&) = delete;
    SpScRingBuffer& operator=(const SpScRingBuffer&) = delete;

    /**
     * @brief Attempt to push an item into the buffer (move)
     * @param item Item to push
     * @return true if successful, false if full
     * @note Hot-path: highly inlined, uses relaxed atomics
     */
    bool tryPush(T&& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        const size_t nextHead = (_head + 1) & (Capacity - 1);
        if (nextHead == _tail) { // buffer full
            _droppedMessageCount.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        _buffer[_head] = std::move(item);
        _head = nextHead;

        _pushedMessageCount.fetch_add(1, std::memory_order_relaxed);
        updateHighWaterMark();
        return true;
    }

    /**
     * @brief Attempt to push an item into the buffer (copy)
     */
    bool tryPush(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const size_t nextHead = (_head + 1) & (Capacity - 1);
        if (nextHead == _tail) { 
            _droppedMessageCount.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        _buffer[_head] = item;
        _head = nextHead;

        _pushedMessageCount.fetch_add(1, std::memory_order_relaxed);
        updateHighWaterMark();
        return true;
    }

    /**
     * @brief Attempt to pop an item from the buffer
     * @param item Reference to store popped item
     * @return true if successful, false if buffer empty
     */
    bool tryPop(T& item) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (_tail == _head) return false; // empty
        item = std::move(_buffer[_tail]);
        _tail = (_tail + 1) & (Capacity - 1);

        _poppedMessageCount.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    bool empty() const noexcept { return _head == _tail; }
    bool full()  const noexcept { return ((_head + 1) & (Capacity - 1)) == _tail; }

    uint64_t droppedMessageCount() const noexcept { return _droppedMessageCount.load(std::memory_order_relaxed); }
    uint64_t pushedMessageCount()  const noexcept { return _pushedMessageCount.load(std::memory_order_relaxed); }
    uint64_t poppedMessageCount()  const noexcept { return _poppedMessageCount.load(std::memory_order_relaxed); }

    size_t size() const noexcept { return (_head - _tail) & (Capacity - 1); }
    size_t highWaterMark() const noexcept { return _highWaterMark.load(std::memory_order_relaxed); }

private:
    alignas(64) T _buffer[Capacity];  ///< Storage aligned to cache line
    alignas(64) size_t _head;         ///< Producer index
    alignas(64) size_t _tail;         ///< Consumer index

    std::atomic<uint64_t> _droppedMessageCount;  ///< Number of dropped messages
    std::atomic<uint64_t> _pushedMessageCount;   ///< Number of successfully pushed messages
    std::atomic<uint64_t> _poppedMessageCount;   ///< Number of successfully popped messages
    std::atomic<size_t>   _highWaterMark;        ///< High-water mark for buffer usage

    /**
     * @brief Update high-water mark
     * @note Only called by producer; relaxed atomics sufficient
     */
    void updateHighWaterMark() noexcept {
        size_t currentSize = (_head - _tail) & (Capacity - 1);
        size_t prevHigh = _highWaterMark.load(std::memory_order_relaxed);

        while (currentSize > prevHigh &&
               !_highWaterMark.compare_exchange_weak(prevHigh, currentSize, std::memory_order_relaxed)) {}
    }
};
