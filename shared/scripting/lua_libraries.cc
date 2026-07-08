#include "shared/pch.hh"

#include "shared/scripting/lua_libraries.hh"

#include "core/utils/physfs.hh"

#include "core/identifier.hh"

#include "shared/mod_context.hh"

static bool run_script(lua_State* L, const ModContext* ctx, std::string_view path) noexcept
{
    auto identifier = Identifier::from_string(path, ctx->name_space());

    if(!identifier.is_valid()) {
        lua_pushfstring(L, "malformed path: %.*s", static_cast<int>(path.size()), path.data());
        return false;
    }

    std::string source;
    auto full_path = identifier.as_file_path("scripts", {});

    if(!utils::read_file(full_path, source)) {
        auto error_view = utils::physfs_error();
        lua_pushfstring(L, "%s: %.*s", full_path.c_str(), static_cast<int>(error_view.size()), error_view.data());
        return false;
    }

    auto chunk_name = std::format("@{}", full_path);
    auto load_status = luaL_loadbuffer(L, source.data(), source.size(), chunk_name.c_str());
    return load_status == LUA_OK;
}

static int api_override_dofile(lua_State* L) noexcept
{
    auto raw_path = luaL_checkstring(L, 1);
    auto path = std::string_view(raw_path, std::strlen(raw_path));
    auto ctx = static_cast<const ModContext*>(lua_touserdata(L, lua_upvalueindex(1)));

    if(!run_script(L, ctx, path)) {
        return lua_error(L);
    }

    lua_call(L, 0, LUA_MULTRET);

    return lua_gettop(L) - 1;
}

static int api_override_print(lua_State* L) noexcept
{
    std::string message;
    message.reserve(512);

    auto argc = lua_gettop(L);

    for(int i = 1; i <= argc; ++i) {
        std::size_t length = 0;
        auto string = luaL_tolstring(L, i, &length);

        if(i > 1)
            message.push_back(char(0x09));
        message.append(string, length);

        lua_pop(L, 1);
    }

    const char* debug_file = "unknown";
    unsigned long debug_line = 0;
    lua_Debug ar;

    if(lua_getstack(L, 1, &ar) && lua_getinfo(L, "Sl", &ar)) {
        debug_file = ar.short_src;
        debug_line = std::max(0, ar.currentline);
    }

    LOG_INFO("{}:{}: {}", debug_file, debug_line, message);

    return 0;
}

void scripting::open_lua_libraries(std::shared_ptr<lua_State>& lua, const ModContext* ctx) noexcept
{
    assert(lua);
    assert(ctx);

    auto L = lua.get();

    luaL_requiref(L, LUA_GNAME, &luaopen_base, 1);
    luaL_requiref(L, LUA_TABLIBNAME, &luaopen_table, 1);
    luaL_requiref(L, LUA_STRLIBNAME, &luaopen_string, 1);
    luaL_requiref(L, LUA_MATHLIBNAME, &luaopen_math, 1);
    luaL_requiref(L, LUA_UTF8LIBNAME, &luaopen_utf8, 1);
    lua_pop(L, 5);

    lua_pushnil(L);
    lua_setglobal(L, "loadfile");

    lua_pushnil(L);
    lua_setglobal(L, "load");

    lua_pushnil(L);
    lua_setglobal(L, "collectgarbage");

    lua_pushlightuserdata(L, const_cast<ModContext*>(ctx));
    lua_pushcclosure(L, &api_override_dofile, 1);
    lua_setglobal(L, "dofile");

    lua_pushlightuserdata(L, const_cast<ModContext*>(ctx));
    lua_pushcclosure(L, &api_override_dofile, 1);
    lua_setglobal(L, "require");

    lua_pushlightuserdata(L, const_cast<ModContext*>(ctx));
    lua_pushcclosure(L, &api_override_print, 1);
    lua_setglobal(L, "print");
}
