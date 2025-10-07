// ---------------------------------------------------------------------------
// @file        pinned_thread.h
// Project     : Beacon
// Component   : HFT / Concurrency
// Description : RAII wrapper for threads optionally pinned to a CPU core
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <thread>
#include <atomic>
#include <iostream>

namespace beacon::hft::concurrency
{

    class PinnedThread
    {
    public:
        template <typename Fn>
        PinnedThread(Fn &&fn, int core, std::atomic<bool> &stopFlag)
        {
            _thread = std::thread([fn = std::forward<Fn>(fn), core, &stopFlag]() mutable
                                  { fn(stopFlag); });
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

    private:
        std::thread _thread;
    };

} // namespace beacon::hft::concurrency
