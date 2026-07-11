#ifndef EFFFA75C_5420_47E3_B0E0_5E7CB636F7E3
#define EFFFA75C_5420_47E3_B0E0_5E7CB636F7E3

#include "core/concepts.hh"

namespace utils
{
template<vx::char_type T>
bool is_whitespace(std::basic_string_view<T> string);
template<vx::char_type T>
bool has_whitespace(std::basic_string_view<T> string);
template<vx::char_type T>
std::basic_string_view<T> skip_whitespace(std::basic_string_view<T> string);
template<vx::char_type T>
std::basic_string_view<T> trim_whitespace(std::basic_string_view<T> string);
template<vx::char_type T>
std::basic_string_view<T> remove_comments(std::basic_string_view<T> string);
template<vx::char_type T>
std::vector<std::basic_string_view<T>> tokenize(std::basic_string_view<T> string);
} // namespace utils

#endif /* EFFFA75C_5420_47E3_B0E0_5E7CB636F7E3 */
