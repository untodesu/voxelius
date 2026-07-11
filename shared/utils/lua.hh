#ifndef B559A84C_2B66_435D_9730_506C892E8E29
#define B559A84C_2B66_435D_9730_506C892E8E29

namespace utils
{
Eigen::Vector2f read_vector2f(lua_State* L, int idx);
Eigen::Vector3f read_vector3f(lua_State* L, int idx);
Eigen::Vector4f read_vector4f(lua_State* L, int idx);
} // namespace utils

namespace utils
{
template<std::unsigned_integral T>
T read_bitmask(lua_State* L, int idx);
} // namespace utils

#endif /* B559A84C_2B66_435D_9730_506C892E8E29 */
