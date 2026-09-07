#ifndef BITCOIN_UTIL_FASTRANGE_H
#define BITCOIN_UTIL_FASTRANGE_H
#include <cstdint>
static inline uint32_t FastRange32(uint32_t x, uint32_t n) { return (uint64_t{x} * uint64_t{n}) >> 32; }
static inline uint64_t FastRange64(uint64_t x, uint64_t n) { return (__uint128_t{x} * __uint128_t{n}) >> 64; }
#endif
