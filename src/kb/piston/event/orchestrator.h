#pragma once

#include "kb/piston/event/event.h"
#include "kb/piston/event/dispatcher.h"
#include "kb/piston/event/meta.h"
#include "kb/piston/meta/meta.h"

#include <variant>
#include <type_traits>

namespace kb::piston::event
{ // start namespace kb::piston::event

namespace details
{ // start namespace ::details

// reference: https://stackoverflow.com/a/78608011
template<class Attribute, class... Attributes>
constexpr std::size_t index_of(const std::tuple<Attributes...>&)
{
    std::size_t i = 0;
    const bool found = ((++i && std::is_same_v < Attribute, Attributes>()) || ...);

    return i - found;
}

template <class T, class Tuple>
struct tuple_index;

template <class T, class... Types>
struct tuple_index<T, std::tuple<T, Types...>>
{
    static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct tuple_index<T, std::tuple<U, Types...>>
{
    static const std::size_t value = 1 + tuple_index<T, std::tuple<Types...>>::value;
};

template <int N, typename... Ts>
struct tuple_get;

template <int N, typename T, typename... Ts>
struct tuple_get<N, std::tuple<T, Ts...>>
{
    using type = typename tuple_get<N - 1, std::tuple<Ts...>>::type;
};

template <typename T, typename... Ts>
struct tuple_get<0, std::tuple<T, Ts...>>
{
    using type = T;
};

} // end namespace ::details

template <meta::PistonEventT... Events>
// TODO: constraint
class orchestrator
{
public:
    using events_t = std::tuple<Events...>;
    using dispatchers_t = std::tuple<dispatcher<typename Events::event_type_t, typename Events::derived_t>...>;

    template <meta::PistonEventT EventT>
    using dispatcher_handler_t = std::function<bool(EventT*)>;

    // TODO: can we get rid of the std::function?
    using handlers_t = std::tuple<
        std::function<bool(typename Events::derived_t*)>...
    >;

    template <typename... Handlers>
    explicit orchestrator(
        Handlers&&... p_handlers
    ) : m_dispatch_handlers{ std::make_tuple(std::forward<Handlers>(p_handlers)...) }
    { }

    template <meta::PistonEventT EventT>
    [[nodiscard]] auto dispatch(EventT& p_event) const noexcept -> bool
    {
        constexpr auto k_event_index = details::tuple_index<EventT, events_t>::value;
        using dispatcher_t = typename details::tuple_get<k_event_index, dispatchers_t>::type;
        return dispatcher_t{ &p_event }.dispatch(std::get<k_event_index>(m_dispatch_handlers));
    }

private:
    handlers_t m_dispatch_handlers{};
};

// Construct an orchestrator from a tuple of types, rather than a variadic template
template <typename T>
// TODO: require T is an std::tuple< ... >
[[nodiscard]] auto make_orchestrator(auto&&... p_handlers) noexcept -> piston::meta::unpack_type_t<T, orchestrator>
{
    return piston::meta::unpack_type_t<T, orchestrator>{ std::forward<decltype(p_handlers)>(p_handlers)... };
}

#if 0
template <typename... Events>
auto make_orchestrator(auto&&... p_handlers) noexcept -> orchestrator<Events...>
{
    return { std::forward<decltype(p_handlers)>(p_handlers)... };
}
#endif

} // end namespace kb::piston::event