#include "shared/pch.hh"

#include "shared/utils/lua.hh"

// NOTE: uses lua_tonumber/lua_tointeger (never errors) rather than the luaL_opt*/luaL_check*
// equivalents, because these are called from deep inside blocks_library.cc's parsing call
// chain while BlockDefinition/BlockDrop/etc (non-trivial destructors) are alive on the C++
// stack; luaL_error()/lua_error() longjmp past those frames, which is UB. A malformed
// numeric field silently reads as 0 instead of aborting the whole parse.

Eigen::Vector2f utils::read_vector2f(lua_State* L, int idx) noexcept
{
    Eigen::Vector2f result = Eigen::Vector2f::Zero();

    lua_rawgeti(L, idx, 1);
    result.x() = static_cast<float>(lua_tonumber(L, -1));

    lua_rawgeti(L, idx, 2);
    result.y() = static_cast<float>(lua_tonumber(L, -1));

    lua_pop(L, 2);

    return result;
}

Eigen::Vector3f utils::read_vector3f(lua_State* L, int idx) noexcept
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

Eigen::Vector4f utils::read_vector4f(lua_State* L, int idx) noexcept
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
T utils::read_bitmask(lua_State* L, int idx) noexcept
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

template unsigned char utils::read_bitmask<unsigned char>(lua_State* L, int idx) noexcept;
template unsigned short utils::read_bitmask<unsigned short>(lua_State* L, int idx) noexcept;
template unsigned utils::read_bitmask<unsigned>(lua_State* L, int idx) noexcept;
template unsigned long utils::read_bitmask<unsigned long>(lua_State* L, int idx) noexcept;
template unsigned long long utils::read_bitmask<unsigned long long>(lua_State* L, int idx) noexcept;
