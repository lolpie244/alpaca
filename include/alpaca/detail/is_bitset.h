#pragma once
#include <bitset>
#include <type_traits>

namespace alpaca {

namespace detail {

// check if T is instantiation of Bitset
template <typename T>
struct is_bitset : std::false_type {};

template <std::size_t N>
struct is_bitset<std::bitset<N>> : std::true_type {};

} // namespace detail

} // namespace alpaca
