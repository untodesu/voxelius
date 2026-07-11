#include "shared/pch.hh"

#include "shared/utils/lua.hh"

Eigen::Vector2f utils::read_vector2f(lua_State* L, int idx)
{
    Eigen::Vector2f result = Eigen::Vector2f::Zero();

    lua_rawgeti(L, idx, 1);
    result.x() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 2);
    result.y() = static_cast<float>(lua_tonumber(L, -1));

    lua_pop(L, 2);

    return result;
}

Eigen::Vector3f utils::read_vector3f(lua_State* L, int idx)
{
    Eigen::Vector3f result = Eigen::Vector3f::Zero();

    lua_rawgeti(L, idx, 1);
    result.x() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 2);
    result.y() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 3);
    result.z() = static_cast<float>(lua_tonumber(L, -1));

    lua_pop(L, 3);

    return result;
}

Eigen::Vector4f utils::read_vector4f(lua_State* L, int idx)
{
    Eigen::Vector4f result = Eigen::Vector4f::Zero();

    lua_rawgeti(L, idx, 1);
    result.x() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 2);
    result.y() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 3);
    result.z() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 4);
    result.w() = static_cast<float>(lua_tonumber(L, -1));

    lua_pop(L, 4);

    return result;
}

template<std::unsigned_integral T>
T utils::read_bitmask(lua_State* L, int idx)
{
    T mask = static_cast<T>(0);
    auto count = lua_rawlen(L, idx);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, idx, i);
        mask |= static_cast<T>(lua_tointeger(L, -1));
        lua_pop(L, 1);
    }

    return mask;
}

template unsigned char utils::read_bitmask<unsigned char>(lua_State* L, int idx);
template unsigned short utils::read_bitmask<unsigned short>(lua_State* L, int idx);
template unsigned utils::read_bitmask<unsigned>(lua_State* L, int idx);
template unsigned long utils::read_bitmask<unsigned long>(lua_State* L, int idx);
template unsigned long long utils::read_bitmask<unsigned long long>(lua_State* L, int idx);
