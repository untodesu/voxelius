#include "shared/pch.hh"

#include "shared/utils/lua.hh"

template<vx::arithmetic T, std::size_t N>
std::optional<Eigen::Vector<T, N>> utils::read_vector(lua_State* L, int idx)
{
    if(lua_istable(L, idx)) {
        Eigen::Vector<T, N> result = Eigen::Vector<T, N>::Zero();

        for(std::size_t i = 0; i < N; ++i) {
            lua_rawgeti(L, idx, static_cast<lua_Integer>(i + 1));

            if constexpr(std::is_integral_v<T>) {
                result[i] = static_cast<T>(lua_tointeger(L, -1));
            }
            else {
                result[i] = static_cast<T>(lua_tonumber(L, -1));
            }

            lua_pop(L, 1);
        }

        return result;
    }

    return std::nullopt;
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

std::optional<lua_Integer> utils::require_integer(lua_State* L, int idx)
{
    int is_num = 0;
    auto value = lua_tointegerx(L, idx, &is_num);

    if(is_num) {
        return value;
    }

    lua_pushfstring(L, "expected an integer, got %s", lua_typename(L, lua_type(L, idx)));

    return std::nullopt;
}

std::optional<lua_Number> utils::require_number(lua_State* L, int idx)
{
    int is_num = 0;
    auto value = lua_tonumberx(L, idx, &is_num);

    if(is_num) {
        return value;
    }

    lua_pushfstring(L, "expected a number, got %s", lua_typename(L, lua_type(L, idx)));

    return std::nullopt;
}

std::optional<std::string_view> utils::require_string(lua_State* L, int idx)
{
    auto type = lua_type(L, idx);

    if(type == LUA_TSTRING || type == LUA_TNUMBER) {
        std::size_t length = 0;
        auto value = lua_tolstring(L, idx, &length);
        return std::string_view(value, length);
    }

    lua_pushfstring(L, "expected a string, got %s", lua_typename(L, type));

    return std::nullopt;
}

std::optional<lua_Integer> utils::opt_integer(lua_State* L, int idx, lua_Integer default_value)
{
    if(lua_isnoneornil(L, idx))
        return default_value;
    return require_integer(L, idx);
}

std::optional<lua_Number> utils::opt_number(lua_State* L, int idx, lua_Number default_value)
{
    if(lua_isnoneornil(L, idx))
        return default_value;
    return require_number(L, idx);
}

std::optional<std::string_view> utils::opt_string(lua_State* L, int idx, std::string_view default_value)
{
    if(lua_isnoneornil(L, idx))
        return default_value;
    return require_string(L, idx);
}

template std::optional<Eigen::Vector<float, 2>> utils::read_vector<float, 2>(lua_State* L, int idx);
template std::optional<Eigen::Vector<float, 3>> utils::read_vector<float, 3>(lua_State* L, int idx);
template std::optional<Eigen::Vector<float, 4>> utils::read_vector<float, 4>(lua_State* L, int idx);

template std::optional<Eigen::Vector<double, 2>> utils::read_vector<double, 2>(lua_State* L, int idx);
template std::optional<Eigen::Vector<double, 3>> utils::read_vector<double, 3>(lua_State* L, int idx);
template std::optional<Eigen::Vector<double, 4>> utils::read_vector<double, 4>(lua_State* L, int idx);

template std::optional<Eigen::Vector<int, 2>> utils::read_vector<int, 2>(lua_State* L, int idx);
template std::optional<Eigen::Vector<int, 3>> utils::read_vector<int, 3>(lua_State* L, int idx);
template std::optional<Eigen::Vector<int, 4>> utils::read_vector<int, 4>(lua_State* L, int idx);

template unsigned char utils::read_bitmask<unsigned char>(lua_State* L, int idx);
template unsigned short utils::read_bitmask<unsigned short>(lua_State* L, int idx);
template unsigned utils::read_bitmask<unsigned>(lua_State* L, int idx);
template unsigned long utils::read_bitmask<unsigned long>(lua_State* L, int idx);
template unsigned long long utils::read_bitmask<unsigned long long>(lua_State* L, int idx);
