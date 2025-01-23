#pragma once

#include <type_traits>
#include <string>

namespace kb::piston
{ // start namespace kb::piston

template <typename ErrorT, typename ErrorV = std::u8string>
requires (std::is_enum_v<ErrorT>)
struct js_error
{
    using error_t = ErrorT;
    using value_t = ErrorV;

    ErrorT m_type;
    ErrorV m_value;

    constexpr js_error() noexcept = default;
    constexpr ~js_error() noexcept = default;

    constexpr js_error(ErrorT p_type) noexcept
        : m_type{ p_type }, m_value{} { }

    constexpr js_error(error_t p_type, value_t p_value) noexcept
        : m_type{ p_type }, m_value{ p_value } { }
};

} // end namespace kb::piston