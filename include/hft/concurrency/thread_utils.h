#include <thread>

#pragma once

namespace beacon::hft::concurrency {
  class ThreadUtils {
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
}

