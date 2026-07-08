#include "shared/pch.hh"

#include "shared/scripting/core_library.hh"

#include "core/config/map.hh"

#include "core/version.hh"

#include "shared/mod_context.hh"

static int api_core_config_has(lua_State* L) noexcept
{
    auto config = static_cast<config::Map*>(lua_touserdata(L, lua_upvalueindex(1)));
    auto key = luaL_checkstring(L, 1);

    lua_pushboolean(L, config->contains(key));
    return 1;
}

static int api_core_config_get(lua_State* L) noexcept
{
    auto config = static_cast<config::Map*>(lua_touserdata(L, lua_upvalueindex(1)));
    auto key = luaL_checkstring(L, 1);

    auto value = config->value<std::string>(key);

    if(value.has_value()) {
        lua_pushlstring(L, value->data(), value->size());
    }
    else {
        lua_pushnil(L);
    }

    return 1;
}

static int api_core_config_set(lua_State* L) noexcept
{
    auto config = static_cast<config::Map*>(lua_touserdata(L, lua_upvalueindex(1)));
    auto key = luaL_checkstring(L, 1);
    auto value = luaL_checkstring(L, 2);

    config->set_value<std::string>(key, std::string(value));

    return 0;
}

void scripting::open_core_library(std::shared_ptr<lua_State>& lua, const ModContext* ctx, config::Map* config) noexcept
{
    assert(lua);
    assert(ctx);
    assert(config);

    auto L = lua.get();

    lua_newtable(L); // core

    lua_pushinteger(L, version::major);
    lua_setfield(L, -2, "VERSION_MAJOR");

    lua_pushinteger(L, version::minor);
    lua_setfield(L, -2, "VERSION_MINOR");

    lua_pushinteger(L, version::patch);
    lua_setfield(L, -2, "VERSION_PATCH");

    lua_pushlstring(L, version::scm_branch.data(), version::scm_branch.size());
    lua_setfield(L, -2, "VERSION_SCM_BRANCH");

    lua_pushlstring(L, version::scm_revision.data(), version::scm_revision.size());
    lua_setfield(L, -2, "VERSION_SCM_REVISION");

    lua_pushlstring(L, version::semantic.data(), version::semantic.size());
    lua_setfield(L, -2, "VERSION_SEMANTIC");

    lua_pushlstring(L, version::full.data(), version::full.size());
    lua_setfield(L, -2, "VERSION");

    auto name_space = ctx->name_space();
    lua_pushlstring(L, name_space.data(), name_space.size());
    lua_setfield(L, -2, "NAMESPACE");

    lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, &api_core_config_has, 1);
    lua_setfield(L, -2, "config_has");

    lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, &api_core_config_get, 1);
    lua_setfield(L, -2, "config_get");

    lua_pushlightuserdata(L, config);
    lua_pushcclosure(L, &api_core_config_set, 1);
    lua_setfield(L, -2, "config_set");

    lua_setglobal(L, "core");
}
