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

namespace beacon::hft::ringbuffer
{
  /** @brief Default capacity for single-producer single-consumer ring buffers */
  inline constexpr int DEFAULT_RING_BUFFER_CAPACITY = 1024;

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
  template <typename T, size_t Capacity = DEFAULT_RING_BUFFER_CAPACITY>
  class SpScRingBuffer {
    public:
      SpScRingBuffer()
          : _head(0), _tail(0), _dropped(0), _highWaterMark(0) {}

      // Non-copyable
      SpScRingBuffer(const SpScRingBuffer &) = delete;
      SpScRingBuffer &operator=(const SpScRingBuffer &) = delete;

      // Non-movable
      SpScRingBuffer(SpScRingBuffer &&) = delete;
      SpScRingBuffer &operator=(SpScRingBuffer &&) = delete;

      /**
       * @brief  Lock-free attempt to push an item onto the ring buffer.
       * @param  Item The item to push onto the ring buffer.
       * @return True if the ring buffer had space for item. False otherwise.
       * @note   There is no alerting if tryPush fails.
       */
      bool tryPush(const T& item) {
        const size_t head = _head.load(std::memory_order_relaxed);
        const size_t next = increment(head);

        if (next == _tail.load(std::memory_order_acquire)) {
          // The buffer is full.
          _dropped.fetch_add(1, std::memory_order_relaxed);
          return false;
        }

        // The buffer has space for the item Insert it.
        _buffer[head] = item;
        _head.store(next, std::memory_order_release);

        // Keep track of the most full the ring buffer has ever been.
        updateHighWaterMark(next);

        return true;
      }

      /**
       * @brief Lock-free attempt to pop an item from the ring buffer.
       * @param item Output parameter to store the popped item.
       * @return True if the ring buffer has items to pop. False otherwise.
       * @note   There is no alerting if tryPush fails.
       */
      bool tryPop(T& item){
        const size_t tail = _tail.load(std::memory_order_relaxed);
        if (tail == _head.load(std::memory_order_acquire)) {
          // The ring buffer is empty.
          return false;
        }

      // Set the out parameter and advance the tail.
      item = _buffer[tail];
      _tail.store(increment(tail), std::memory_order_release);

        return true;
      }

      /**
       * @brief Returns total number of lost due to the ring buffer being full.
       */
      size_t dropped() const {
        return _dropped.load(std::memory_order_relaxed);
      }

      /**
       * @brief Returns the most full that the ring buffer has ever been,.
       */
      size_t highWaterMark() const {
        return _highWaterMark.load(std::memory_order_relaxed);
      }

  private:
      std::array<T, Capacity> _buffer;
      alignas(64) std::atomic<size_t> _head;
      alignas(64) std::atomic<size_t> _tail;
      std::atomic<size_t> _dropped;
      std::atomic<size_t> _highWaterMark;

      size_t increment(size_t idx) const noexcept { return (idx + 1) % Capacity; }

      /**
       * @brief Maintains the most full that the fing buffer has ever been.
       * @param head The current head index used to measure the ring buffer size.
       * @note  Be cautious as we use the modulo operator.
       */
      void updateHighWaterMark(size_t head) {
          size_t current = _highWaterMark.load(std::memory_order_relaxed);
          size_t used = (head + Capacity - _tail.load(std::memory_order_relaxed)) % Capacity;
          while (used > current && !_highWaterMark.compare_exchange_weak(current, used, std::memory_order_relaxed))
          {
          }
      }
    };  //End class SpScRingBuffer
  } //End namespace beacon::hft::ringbuffer