#pragma once

namespace kb::piston::meta
{ // start namespace kb::piston::meta

template <template <typename...> typename Target, typename Tuple>
struct unpack_tuple;

template <template <typename...> typename Target, typename... Types>
struct unpack_tuple<Target, std::tuple<Types...>>
{
    using type = Target<Types...>;
};

template <typename TupleT, template <typename ...> typename Target>
using unpack_type_t = typename unpack_tuple<Target, TupleT>::type;

// reference: https://stackoverflow.com/a/72162672
template<typename Types, typename Functor, typename... Args>
constexpr void for_each(Functor&& f, Args&&... args)
{
    [&] <template<typename...> class TL, typename... Ts>(TL<Ts...>*)
    {
        ((void)f.template operator() < Ts > (std::forward<Args>(args)...), ...);
    }(static_cast<Types*>(nullptr));
}

} // end namespace kb::piston::meta