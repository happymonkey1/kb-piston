#pragma once
#include <type_traits>

namespace kb::piston::event
{ // start namespace kb::piston::event

template <typename EventT, typename DerivedT>
requires (std::is_enum_v<EventT>)
class piston_event
{
public:
    using event_type_t = EventT;
    using derived_t = DerivedT;

public:
    piston_event() = default;
    ~piston_event() = default;

    static auto get_event_type() noexcept -> event_type_t { return derived().get_event_type_impl(); }

    auto get_piston_event() const noexcept -> const piston_event& { return derived().get_piston_event_impl(); }
    auto get_piston_event() noexcept -> piston_event& { return derived().get_piston_event_impl(); }

    auto get_event() const noexcept -> const derived_t& { return derived().get_event_impl(); }
    auto get_event() noexcept -> derived_t& { return derived().get_event_impl(); }

private:
    auto derived() const noexcept -> const derived_t& { return *static_cast<derived_t*>(this); }
    auto derived() noexcept -> derived_t& { return *static_cast<derived_t*>(this); }
};

#define PISTON_EVENT_BEGIN(derived_event, event_type, js_event_func_name) \
    class derived_event : public kb::piston::event::piston_event<event_type_t, derived_event> { \
    public: \
    derived_event() noexcept = default; \
    ~derived_event() noexcept = default; \
    using piston_event::derived_t; \
    using piston_event::event_type_t; \
    static auto get_event_type_impl() noexcept -> decltype(event_type) { return event_type; } \
    auto get_piston_event_impl() const noexcept -> const piston_event& { return *this; } \
    auto get_piston_event_impl() noexcept -> piston_event& { return *this; } \
    auto get_event_impl() const noexcept -> const derived_event& { return *this; } \
    auto get_event_impl() noexcept -> derived_event& { return *this; } \
    constexpr static const char (&get_js_event_function_name())[sizeof(js_event_func_name)] { return { js_event_func_name }; } \
    private:

#define PISTON_EVENT_END() };

} // start namespace kb::piston::event