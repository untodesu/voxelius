#include "shared/pch.hh"

#include "shared/scripting/sandbox.hh"

void scripting::open_sandboxed_libs(std::shared_ptr<lua_State>& lua) noexcept
{
    constexpr static std::array safe_libraries = {
        std::make_pair(LUA_GNAME, &luaopen_base),
        std::make_pair(LUA_TABLIBNAME, &luaopen_table),
        std::make_pair(LUA_STRLIBNAME, &luaopen_string),
        std::make_pair(LUA_MATHLIBNAME, &luaopen_math),
        std::make_pair(LUA_UTF8LIBNAME, &luaopen_utf8),
    };

    constexpr static std::array<const char*, 5> strip_globals = {
        "dofile",
        "loadfile",
        "load",
        "require",
        "print",
    };

    auto L = lua.get();

    for(const auto& library : safe_libraries) {
        luaL_requiref(L, library.first, library.second, 1);
        lua_pop(L, 1);
    }

    for(const auto name : strip_globals) {
        lua_pushnil(L);
        lua_setglobal(L, name);
    }
}
