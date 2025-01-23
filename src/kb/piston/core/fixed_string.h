#pragma once

#include <cstdint>
#include <array>
#include <string_view>

namespace kb::piston
{ // start namespace kb::piston

template <typename T, size_t Capacity>
class fixed_string
{
public:
    using char_t = T;

public:
    constexpr fixed_string() noexcept = default;
    constexpr ~fixed_string() noexcept = default;

    constexpr fixed_string(const fixed_string&) noexcept = default;
    constexpr auto operator=(const fixed_string&) noexcept -> fixed_string& = default;

    constexpr fixed_string(fixed_string&&) noexcept = default;
    constexpr auto operator=(fixed_string&&) noexcept -> fixed_string& = default;

    template <size_t Size>
    constexpr fixed_string(const char_t(&p_str)[Size])
    {
        std::memcpy(data(), p_str, Size);
    }

    constexpr fixed_string(std::string_view p_str)
    {
        std::memcpy(data(), p_str.data(), p_str.size());
    }

    [[nodiscard]] constexpr auto data() noexcept -> char_t* { return m_data.data(); }
    [[nodiscard]] constexpr auto data() const noexcept -> const char_t* { return m_data.data(); }

    [[nodiscard]] constexpr auto size() const noexcept -> size_t { return m_data.size(); }

private:
    std::array<char_t, Capacity> m_data{};
};

} // end namespace kb::piston