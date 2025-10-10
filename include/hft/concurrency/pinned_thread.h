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
        auto threadFn = [&fnLocal, this]() {  // capture by reference so we have access to _stopFlag
          fnLocal(_stopFlag);                 // invoke the user's function with atomic _stopFlag
        };
        _thread = std::thread(threadFn);      // start the thread which now has _stopFlag
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