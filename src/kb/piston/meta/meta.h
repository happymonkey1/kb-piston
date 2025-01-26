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

} // end namespace kb::piston::meta