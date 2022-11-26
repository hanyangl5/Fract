#pragma once

#include <cstdint>
#include "container/Container.h"
#include "memory/Memory.h"

namespace Fract {

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

template<typename T> bool Min(const T& a, const T& b) { return a < b ? a : b; }

template <typename T> bool Max(const T &a, const T &b) { return a > b ? a : b; }

    template <typename T> __forceinline T AlignUpWithMask(T value, size_t mask) {
    return (T)(((size_t)value + mask) & ~mask);
}

template <typename T> __forceinline T AlignDownWithMask(T value, size_t mask) {
    return (T)((size_t)value & ~mask);
}

template <typename T> __forceinline T AlignUp(T value, size_t alignment) {
    return AlignUpWithMask(value, alignment - 1);
}

template <typename T> __forceinline T AlignDown(T value, size_t alignment) {
    return AlignDownWithMask(value, alignment - 1);
}

template <typename T> __forceinline bool IsAligned(T value, size_t alignment) {
    return 0 == ((size_t)value & (alignment - 1));
}

template <typename T>
__forceinline T DivideByMultiple(T value, size_t alignment) {
    return (T)((value + alignment - 1) / alignment);
}

template <typename T> __forceinline bool IsPowerOfTwo(T value) {
    return 0 == (value & (value - 1));
}

template <typename T> __forceinline bool IsDivisible(T value, T divisor) {
    return (value / divisor) * divisor == value;
}

__forceinline uint8_t Log2(uint64_t value) {
    unsigned long mssb; // most significant set bit
    unsigned long lssb; // least significant set bit

    // If perfect power of two (only one set bit), return index of bit.
    // Otherwise round up fractional log by adding 1 to most signicant set bit's
    // index.
    if (_BitScanReverse64(&mssb, value) > 0 &&
        _BitScanForward64(&lssb, value) > 0)
        return uint8_t(mssb + (mssb == lssb ? 0 : 1));
    else
        return 0;
}

template <typename T> __forceinline T AlignPowerOfTwo(T value) {
    return value == 0 ? 0 : 1 << Log2(value);
}

} // namespace Ori
