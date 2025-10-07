// ---------------------------------------------------------------------------
// @file        hton_utils.h
// Project     : Beacon
// Component   : Common
// Description : Cross-platform helpers for 64-bit host/network byte order conversion
// Author      : Bryan Camp
// ---------------------------------------------------------------------------

#pragma once

#include <arpa/inet.h>

#include <cstdint>

namespace beacon::hft::networking {

/**
 * @brief Swap bytes of a 64-bit integer
 * @param x 64-bit value to swap
 * @return byte-swapped 64-bit value
 */
inline uint64_t bswap64(uint64_t x) {
    return ((x & 0xFF00000000000000ULL) >> 56) | ((x & 0x00FF000000000000ULL) >> 40) |
           ((x & 0x0000FF0000000000ULL) >> 24) | ((x & 0x000000FF00000000ULL) >> 8) |
           ((x & 0x00000000FF000000ULL) << 8) | ((x & 0x0000000000FF0000ULL) << 24) |
           ((x & 0x000000000000FF00ULL) << 40) | ((x & 0x00000000000000FFULL) << 56);
}

#if defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#undef htonll
#undef ntohll

inline uint64_t htonll(uint64_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return OSSwapHostToBigInt64(x);
#else
    return x;
#endif
}

inline uint64_t ntohll(uint64_t x) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return OSSwapBigToHostInt64(x);
#else
    return x;
#endif
}

#else  // Linux / other platforms

inline uint64_t htonll(uint64_t x) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return bswap64(x);
#else
    return x;
#endif
}

inline uint64_t ntohll(uint64_t x) {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return bswap64(x);
#else
    return x;
#endif
}

#endif  // platform check

}  // namespace beacon::hft::networking
