#ifndef B229B29C_CA21_4217_81CB_8DD0C8FC4BC4
#define B229B29C_CA21_4217_81CB_8DD0C8FC4BC4

#include "core/concepts.hh"

namespace utils
{
template<vx::arithmetic T, std::size_t N>
bool parse_json(const JSON_Value* value, Eigen::Vector<T, N>& vector) noexcept;
template<vx::arithmetic T, std::size_t N>
bool parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<T, N>& vector) noexcept;
} // namespace utils

#endif /* B229B29C_CA21_4217_81CB_8DD0C8FC4BC4 */
