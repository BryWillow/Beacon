#pragma once
#include <atomic>
#include <cstddef>

template <typename T, size_t N>
class SpScRingBuffer {
public:
    bool tryPop(T&) { return false; } ///< Placeholder
};
