#include "shared/pch.hh"

#include "shared/scripting/world_library.hh"

#include "shared/utils/lua.hh"

#include "shared/world.hh"

static int api_get_block(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto block_id = world::get_block(pos);

    lua_pushinteger(L, static_cast<lua_Integer>(block_id));
    return 1;
}

static int api_set_block(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto block_id = static_cast<block_id_type>(luaL_checkinteger(L, 4));
    auto success = world::set_block(pos, block_id);

    lua_pushboolean(L, success);
    return 1;
}

static int api_get_light(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto light = world::get_light(pos);

    lua_pushinteger(L, static_cast<lua_Integer>(light));
    return 1;
}

static int api_get_state(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto state_name = luaL_checkstring(L, 4);
    auto value_opt = world::get_state(pos, state_name);

    if(value_opt.has_value()) {
        lua_pushlstring(L, value_opt->data(), value_opt->size());
        return 1;
    }
    else {
        lua_pushnil(L);
        return 1;
    }
}

static int api_set_state(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto state_name = luaL_checkstring(L, 4);
    auto state_value = luaL_checkstring(L, 5);

    auto success = world::set_state(pos, state_name, state_value);

    lua_pushboolean(L, success);
    return 1;
}

static int api_get_temperature_base(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto temperature = world::get_temperature_base(pos);

    lua_pushinteger(L, static_cast<lua_Integer>(temperature));
    return 1;
}

static int api_get_temperature(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto temperature = world::get_temperature(pos);

    lua_pushinteger(L, static_cast<lua_Integer>(temperature));
    return 1;
}

static int api_schedule(lua_State* L)
{
    block_pos pos = block_pos::Zero();
    pos.x() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 1));
    pos.y() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 2));
    pos.z() = static_cast<block_pos::value_type>(luaL_checkinteger(L, 3));

    auto deadline = static_cast<std::uint64_t>(luaL_checkinteger(L, 4));

    world::schedule(pos, deadline);

    return 0;
}

void scripting::open_world_library(std::shared_ptr<lua_State>& lua, const ModContext* ctx)
{
    assert(lua);
    assert(ctx);

    auto L = lua.get();

    lua_newtable(L); // world

    lua_pushcfunction(L, api_get_block);
    lua_setfield(L, -2, "get_block");

    lua_pushcfunction(L, api_set_block);
    lua_setfield(L, -2, "set_block");

    lua_pushcfunction(L, api_get_light);
    lua_setfield(L, -2, "get_light");

    lua_pushcfunction(L, api_get_state);
    lua_setfield(L, -2, "get_state");

    lua_pushcfunction(L, api_set_state);
    lua_setfield(L, -2, "set_state");

    lua_pushcfunction(L, api_get_temperature_base);
    lua_setfield(L, -2, "get_temperature_base");

    lua_pushcfunction(L, api_get_temperature);
    lua_setfield(L, -2, "get_temperature");

    lua_pushcfunction(L, api_schedule);
    lua_setfield(L, -2, "schedule");

    lua_setglobal(L, "world");
}
