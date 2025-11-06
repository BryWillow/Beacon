// ---------------------------------------------------------------------------
// @file        pinned_thread.h
// Project     : Beacon
// Component   : HFT / Concurrency
// Description : Pins an std::thread to a specific core. RAII-friendly.
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <thread>
#include <atomic>
#include <iostream>
#include "thread_utils.h"

namespace beacon::hft::concurrency
{
  class PinnedThread
  {
    public:
      template <typename Fn>
      PinnedThread(Fn&& fn, int core) {
        auto fnLocal = std::forward<Fn>(fn);  // take ownership of the user-provided function
        auto threadFn = [fnLocal = std::move(fnLocal), this]() mutable {  // capture by value (move), mutable so we can call it
          fnLocal(_stopFlag);                 // invoke the user's function with atomic _stopFlag
        };
        _thread = std::thread(std::move(threadFn));      // start the thread which now has _stopFlag
        ThreadUtils::pinThreadToCore(_thread, core); // pin the thread to the specified core
      }

      ~PinnedThread() {
        if (_thread.joinable())
          _thread.join();
      }

      void stop() {
        _stopFlag.store(true, std::memory_order_relaxed);
        if (_thread.joinable())
          _thread.join();
      }

    private:
      std::atomic<bool> _stopFlag;
      std::thread _thread;
  };
} // namespace beacon::hft::concurrency