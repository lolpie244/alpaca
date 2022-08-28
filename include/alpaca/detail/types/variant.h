#pragma once
#include <alpaca/detail/type_info.h>
#include <alpaca/detail/variable_length_encoding.h>
#include <alpaca/detail/variant_nth_field.h>
#include <system_error>
#include <variant>
#include <vector>

namespace alpaca {

namespace detail {

template <typename T, 
          std::size_t N = detail::aggregate_arity<std::remove_cv_t<T>>::size()>
typename std::enable_if<std::is_aggregate_v<T>, void>::type
type_info(std::vector<uint8_t>& typeids, 
  std::unordered_map<std::string_view, std::size_t>& struct_visitor_map);

template <typename T, std::size_t N, std::size_t I>
void type_info_variant_helper(std::vector<uint8_t>& typeids, 
  std::unordered_map<std::string_view, std::size_t>& struct_visitor_map) {
  if constexpr (I < N) {

    // save current type
    type_info<std::variant_alternative_t<I, T>>(typeids, struct_visitor_map);

    // go to next type
    type_info_variant_helper<T, N, I+1>(typeids, struct_visitor_map);
  }
}

template <typename T>
typename std::enable_if<is_specialization<T, std::variant>::value, void>::type
type_info(std::vector<uint8_t>& typeids, 
  std::unordered_map<std::string_view, std::size_t>& struct_visitor_map) {
  typeids.push_back(to_byte<field_type::variant>());
  constexpr auto variant_size = std::variant_size_v<T>;
  type_info_variant_helper<T, variant_size, 0>(typeids, struct_visitor_map);
}

template <typename T>
void to_bytes_router(const T &input, std::vector<uint8_t> &bytes);

template <typename T, typename... U>
void to_bytes(T &bytes, const std::variant<U...> &input) {
    std::size_t index = input.index();

    // save index of variant
    to_bytes_router<std::size_t>(index, bytes);

    // save value of variant
    const auto visitor = [&bytes](auto &&arg) { to_bytes_router(arg, bytes); };
    std::visit(visitor, input);
}

template <typename... T>
bool from_bytes(std::variant<T...> &output, const std::vector<uint8_t> &bytes,
            std::size_t &byte_index,
            std::error_code &error_code) {
    // current byte is the index of the variant value
    std::size_t index = detail::decode_varint<std::size_t>(bytes, byte_index);

    // read bytes as value_type = variant@index
    detail::set_variant_value<std::variant<T...>>(output, index, bytes, byte_index, error_code);

    return true;
}

}

}