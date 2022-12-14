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

} // namespace Ori
