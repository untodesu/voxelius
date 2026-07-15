#ifndef B559A84C_2B66_435D_9730_506C892E8E29
#define B559A84C_2B66_435D_9730_506C892E8E29

#include "core/concepts.hh"

namespace utils
{
template<vx::arithmetic T, std::size_t N>
std::optional<Eigen::Vector<T, N>> read_vector(lua_State* L, int idx);
} // namespace utils

namespace utils
{
template<std::unsigned_integral T>
T read_bitmask(lua_State* L, int idx);
} // namespace utils

namespace utils
{
std::optional<lua_Integer> require_integer(lua_State* L, int idx);
std::optional<lua_Number> require_number(lua_State* L, int idx);
std::optional<std::string_view> require_string(lua_State* L, int idx);
} // namespace utils

namespace utils
{
std::optional<lua_Integer> opt_integer(lua_State* L, int idx, lua_Integer default_value);
std::optional<lua_Number> opt_number(lua_State* L, int idx, lua_Number default_value);
std::optional<std::string_view> opt_string(lua_State* L, int idx, std::string_view default_value);
} // namespace utils

#endif /* B559A84C_2B66_435D_9730_506C892E8E29 */
