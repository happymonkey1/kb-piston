#pragma once
#include "kb/piston/event/event.h"

namespace kb::piston::event
{ // start namespace kb::piston::event

// Could this just be a function?
/*
 * template <typename EventT, typename DerivedT
 * auto dispatch(piston_event<EventT, DerivedT> p_event, auto&& p_on_dispatch_func) noexcept -> bool;
 */

template <typename EventT, typename DerivedT>
    requires (std::is_base_of_v<piston_event<EventT, DerivedT>, DerivedT>)
class dispatcher
{
public:
    using event_type_t = EventT;
    using derived_type_t = DerivedT;
    using event_t = piston_event<event_type_t, derived_type_t>;

public:
    dispatcher() = delete;

    // Default copy constructor
    dispatcher(const dispatcher&) noexcept = default;
    // Default copy assign operator
    auto operator=(const dispatcher&) noexcept -> dispatcher& = default;
    // Default move constructor
    dispatcher(dispatcher&&) noexcept = default;
    // Default move assign operator
    auto operator=(dispatcher&&) noexcept -> dispatcher& = default;

    explicit dispatcher(event_t* KB_RESTRICT p_event) noexcept
        : m_event{ p_event }
    {
        KB_PISTON_ASSERT(p_event, "[dispatcher]: Event can not be null!");
    }

    ~dispatcher() noexcept = default;

    // Dispatch a handler for the event
    auto dispatch(auto&& p_on_dispatch_func) noexcept -> bool { return p_on_dispatch_func(&m_event->get_event()); }

private:
    event_t* m_event;
};

} // end namespace kb::piston::event