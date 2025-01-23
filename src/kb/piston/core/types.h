#pragma once

#include "kb/piston/core/fixed_string.h"

#ifndef KB_ENGINE

#include <cstdint>
#include <optional>

namespace kb::piston
{ // start namespace kb::piston

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

template <typename T>
using option = std::optional<T>;

using time_step_t = f32;

template <size_t Capacity>
using u8_fixed_string = fixed_string<u8, Capacity>;

template <size_t Capacity>
using i8_fixed_string = fixed_string<i8, Capacity>;

} // end namespace kb::piston

#else
#   include <Kablunk/Core/CoreTypes.h>
#   include <Kablunk/Core/Timestep.h>

namespace kb::piston
{ // start namespace kb::piston

using time_step_t = kb::Timestep;

} // end namespace kb::piston

#endif
