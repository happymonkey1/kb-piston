#pragma once
#include <type_traits>

#include "kb/piston/event/event.h"

namespace kb::piston::event::meta
{ // start namespace kb::piston::event::meta

template <typename T>
struct is_piston_event
{
    static const bool value = std::is_base_of_v<piston_event<typename T::event_type_t, T>, T>;
};

template <typename T>
constexpr auto is_piston_event_v = is_piston_event<T>::value;

template <typename T>
concept PistonEventT = is_piston_event_v<T>;

} // end namespace kb::piston::event::meta