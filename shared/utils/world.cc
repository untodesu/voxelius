#include "shared/pch.hh"

#include "shared/utils/world.hh"

#include "core/utils/crc64.hh"

#include "shared/block_registry.hh"
#include "shared/physics.hh"
#include "shared/world.hh"

static bool call_routine(lua_State* L, int argc, int retc, std::string_view debug_name)
{
    auto status = lua_pcall(L, argc, retc, 0);

    if(status == LUA_OK) {
        return true;
    }

    LOG_ERROR("{}: {}", debug_name, lua_tostring(L, -1));
    lua_pop(L, 1);
    return false;
}

bool utils::block_place(const physics::BlockHit& hit, entt::entity actor, block_id_type id) noexcept
{
    auto family = block_registry::find_family_of(id);
    auto pos = hit.block_pos + hit.normal.cast<BlockPos::value_type>();

    if(family == nullptr) {
        return world::set_block(pos, id);
    }

    const auto& callback = family->on_place;

    if(callback.callback_ref == LUA_NOREF || callback.state == nullptr) {
        return world::set_block(pos, id);
    }

    auto L = callback.state.get();
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback.callback_ref);

    lua_pushinteger(L, static_cast<lua_Integer>(pos.x()));
    lua_pushinteger(L, static_cast<lua_Integer>(pos.y()));
    lua_pushinteger(L, static_cast<lua_Integer>(pos.z()));

    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(hit.id));
    lua_setfield(L, -2, "id");
    lua_pushinteger(L, static_cast<lua_Integer>(hit.face));
    lua_setfield(L, -2, "face");
    lua_pushnumber(L, static_cast<lua_Number>(hit.normal.x()));
    lua_setfield(L, -2, "ni");
    lua_pushnumber(L, static_cast<lua_Number>(hit.normal.y()));
    lua_setfield(L, -2, "nj");
    lua_pushnumber(L, static_cast<lua_Number>(hit.normal.z()));
    lua_setfield(L, -2, "nk");
    lua_pushnumber(L, static_cast<lua_Number>(hit.point.x()));
    lua_setfield(L, -2, "lx");
    lua_pushnumber(L, static_cast<lua_Number>(hit.point.y()));
    lua_setfield(L, -2, "ly");
    lua_pushnumber(L, static_cast<lua_Number>(hit.point.z()));
    lua_setfield(L, -2, "lz");
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.x()));
    lua_setfield(L, -2, "bx");
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.y()));
    lua_setfield(L, -2, "by");
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.z()));
    lua_setfield(L, -2, "bz");
    lua_pushnumber(L, static_cast<lua_Number>(std::fmod(hit.point.x(), 1.0f)));
    lua_setfield(L, -2, "rx");
    lua_pushnumber(L, static_cast<lua_Number>(std::fmod(hit.point.y(), 1.0f)));
    lua_setfield(L, -2, "ry");
    lua_pushnumber(L, static_cast<lua_Number>(std::fmod(hit.point.z(), 1.0f)));
    lua_setfield(L, -2, "rz");

    lua_pushinteger(L, static_cast<lua_Integer>(actor));

    if(!call_routine(L, 5, 1, family->name.full_string())) {
        return false;
    }

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    if(lua_istable(L, -1)) {
        emhash8::HashMap<blockstate_key_type, blockstate_val_type> states;

        for(const auto& it : family->states) {
            states.insert_or_assign(blockstate_key_type(it.first), blockstate_val_type(it.second.default_value));
        }

        lua_pushnil(L);

        while(lua_next(L, -2)) {
            if(lua_isstring(L, -2) && lua_isstring(L, -1)) {
                std::size_t key_length;
                std::size_t val_length;
                const auto key_cstr = lua_tolstring(L, -2, &key_length);
                const auto val_cstr = lua_tolstring(L, -1, &val_length);

                auto key = static_cast<blockstate_key_type>(utils::crc64(std::span(key_cstr, key_length)));

                if(family->states.contains(key)) {
                    auto val = family->state_hash(std::string_view(val_cstr, val_length));

                    states.insert_or_assign(key, blockstate_val_type(val));
                }
            }

            lua_pop(L, 1);
        }

        id = block_registry::resolve_variant(family->stem_id, states);
    }

    return world::set_block(pos, id);
}

bool utils::block_break(const physics::BlockHit& hit, entt::entity actor) noexcept
{
    auto replace_block = BLOCK_ID_NULL; // TODO: ask biome subsystem for this
    auto family = block_registry::find_family_of(hit.id);

    if(family == nullptr) {
        return world::set_block(hit.block_pos, replace_block);
    }

    const auto& callback = family->on_break;

    if(callback.callback_ref == LUA_NOREF || callback.state == nullptr) {
        return world::set_block(hit.block_pos, replace_block);
    }

    auto L = callback.state.get();
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback.callback_ref);

    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.x()));
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.y()));
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.z()));
    lua_pushinteger(L, static_cast<lua_Integer>(actor));

    call_routine(L, 4, 1, family->name.full_string());

    return world::set_block(hit.block_pos, replace_block);
}

bool utils::block_interact(const physics::BlockHit& hit, entt::entity actor) noexcept
{
    auto family = block_registry::find_family_of(hit.id);

    if(family == nullptr) {
        return false;
    }

    const auto& callback = family->on_interact;

    if(callback.callback_ref == LUA_NOREF || callback.state == nullptr) {
        return false;
    }

    auto L = callback.state.get();
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback.callback_ref);

    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.x()));
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.y()));
    lua_pushinteger(L, static_cast<lua_Integer>(hit.block_pos.z()));

    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(hit.face));
    lua_setfield(L, -2, "face");
    lua_pushnumber(L, static_cast<lua_Number>(hit.normal.x()));
    lua_setfield(L, -2, "ni");
    lua_pushnumber(L, static_cast<lua_Number>(hit.normal.y()));
    lua_setfield(L, -2, "nj");
    lua_pushnumber(L, static_cast<lua_Number>(hit.normal.z()));
    lua_setfield(L, -2, "nk");
    lua_pushnumber(L, static_cast<lua_Number>(hit.point.x()));
    lua_setfield(L, -2, "lx");
    lua_pushnumber(L, static_cast<lua_Number>(hit.point.y()));
    lua_setfield(L, -2, "ly");
    lua_pushnumber(L, static_cast<lua_Number>(hit.point.z()));
    lua_setfield(L, -2, "lz");
    lua_pushinteger(L, static_cast<lua_Integer>(std::fmod(hit.point.x(), 1.0f)));
    lua_setfield(L, -2, "rx");
    lua_pushinteger(L, static_cast<lua_Integer>(std::fmod(hit.point.y(), 1.0f)));
    lua_setfield(L, -2, "ry");
    lua_pushinteger(L, static_cast<lua_Integer>(std::fmod(hit.point.z(), 1.0f)));
    lua_setfield(L, -2, "rz");

    lua_pushinteger(L, static_cast<lua_Integer>(actor));

    return call_routine(L, 5, 1, family->name.full_string());
}

bool utils::block_random_tick(const BlockPos& pos) noexcept
{
    auto id = world::get_block(pos);
    auto family = block_registry::find_family_of(id);

    if(family == nullptr) {
        return false;
    }

    const auto& callback = family->on_rtick;

    if(callback.callback_ref == LUA_NOREF || callback.state == nullptr) {
        return false;
    }

    auto L = callback.state.get();
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback.callback_ref);

    lua_pushinteger(L, static_cast<lua_Integer>(pos.x()));
    lua_pushinteger(L, static_cast<lua_Integer>(pos.y()));
    lua_pushinteger(L, static_cast<lua_Integer>(pos.z()));

    return call_routine(L, 3, 1, family->name.full_string());
}

bool utils::block_sched_tick(const BlockPos& pos) noexcept
{
    auto id = world::get_block(pos);
    auto family = block_registry::find_family_of(id);

    if(family == nullptr) {
        return false;
    }

    const auto& callback = family->on_stick;

    if(callback.callback_ref == LUA_NOREF || callback.state == nullptr) {
        return false;
    }

    auto L = callback.state.get();
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback.callback_ref);

    lua_pushinteger(L, static_cast<lua_Integer>(pos.x()));
    lua_pushinteger(L, static_cast<lua_Integer>(pos.y()));
    lua_pushinteger(L, static_cast<lua_Integer>(pos.z()));

    return call_routine(L, 3, 1, family->name.full_string());
}
