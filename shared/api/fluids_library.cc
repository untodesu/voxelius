#include "shared/pch.hh"

#include "shared/api/fluids_library.hh"

#include "core/identifier.hh"

#include "shared/mod_context.hh"
#include "shared/utils/lua.hh"
#include "shared/world/fluid_registry.hh"

static Identifier make_unique_id(const Identifier& id, const ModContext* ctx)
{
    auto is_free = [&](const Identifier& candidate) {
        return ctx->find_fluid(candidate) == FLUID_ID_NULL && fluid_registry::find(candidate) == FLUID_ID_NULL;
    };

    if(is_free(id)) {
        return id;
    }

    for(unsigned n = 1; n < 10000; ++n) {
        auto candidate = Identifier::from_parts(id.name_space(), std::format("{}~{}", id.value(), n));

        if(is_free(candidate)) {
            LOG_WARNING("fluids.add: {} is already registered, using {} instead", id.full_string(), candidate.full_string());
            return candidate;
        }
    }

    LOG_CRITICAL("fluids.add: {}: out of resolvable names", id.full_string());
    return id;
}

static bool parse_textures(lua_State* L, int idx, std::vector<Identifier>& out, ModContext* ctx)
{
    if(lua_isnil(L, idx)) {
        out.clear();
        return true;
    }

    if(!lua_istable(L, idx)) {
        lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, idx)));
        return false;
    }

    out.clear();

    lua_pushnil(L);

    while(lua_next(L, idx)) {
        auto value_str = utils::require_string(L, -1);

        if(!value_str.has_value()) {
            return false;
        }

        auto id = Identifier::from_string(value_str.value(), ctx->name_space());

        if(!id.is_valid()) {
            lua_pushfstring(L, "malformed texture name: %s", value_str.value());
            return false;
        }

        out.push_back(std::move(id));

        lua_pop(L, 1);
    }

    return true;
}

static bool parse_definition(lua_State* L, int def_idx, FluidDefinition& def, ModContext* ctx)
{
    lua_getfield(L, def_idx, "gravity");
    auto gravity = utils::require_integer(L, -1);

    if(!gravity.has_value()) {
        return false;
    }

    def.gravity = static_cast<fluid_gravity>(gravity.value());
    lua_pop(L, 1);

    lua_getfield(L, def_idx, "opaque");

    if(!lua_isnil(L, -1)) {
        if(!lua_isboolean(L, -1)) {
            lua_pushfstring(L, "expected a boolean, got %s", lua_typename(L, lua_type(L, -1)));
            return false;
        }

        def.opaque = lua_toboolean(L, -1);
    }

    lua_pop(L, 1);

    lua_getfield(L, def_idx, "max_level");

    if(!lua_isnoneornil(L, -1)) {
        auto max_level = utils::require_integer(L, -1);

        if(!max_level.has_value()) {
            return false;
        }

        def.max_level = static_cast<unsigned>(max_level.value());
    }

    lua_pop(L, 1);

    lua_getfield(L, def_idx, "tint");
    auto tint = utils::opt_integer(L, -1, 0);

    if(tint.has_value()) {
        def.tint = static_cast<unsigned>(tint.value());
    }

    lua_pop(L, 1);

    lua_getfield(L, def_idx, "textures");

    if(lua_isnil(L, -1)) {
        lua_pushfstring(L, "missing required textures field");
        return false;
    }

    if(!lua_istable(L, -1)) {
        lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, -1)));
        return false;
    }

    lua_getfield(L, -1, "still");

    if(!parse_textures(L, lua_gettop(L), def.still_textures, ctx)) {
        return false;
    }

    lua_pop(L, 1);

    if(def.still_textures.empty()) {
        lua_pushfstring(L, "missing required still textures");
        return false;
    }

    lua_getfield(L, -1, "flowing");

    if(!parse_textures(L, lua_gettop(L), def.flowing_textures, ctx)) {
        return false;
    }

    lua_pop(L, 1);

    if(def.flowing_textures.empty()) {
        lua_pushfstring(L, "missing required flowing textures");
        return false;
    }

    lua_pop(L, 1); // textures
    return true;
}

static bool add_fluid(lua_State* L, ModContext* ctx, const char* raw_name, int def_idx, fluid_id_type& out_fluid_id)
{
    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(!id.is_valid()) {
        lua_pushfstring(L, "fluids.add: malformed name: %s", raw_name);
        return false;
    }

    id = make_unique_id(id, ctx);

    FluidDefinition def {};

    def.opaque = false;
    def.max_level = std::nullopt;
    def.tint = 0;

    if(!parse_definition(L, def_idx, def, ctx)) {
        return false;
    }

    out_fluid_id = ctx->register_fluid(id, std::move(def));

    return true;
}

static int api_add(lua_State* L)
{
    auto ctx = static_cast<ModContext*>(lua_touserdata(L, lua_upvalueindex(1)));

    auto raw_name = luaL_checkstring(L, 1);

    int def_idx = 2;

    luaL_checktype(L, def_idx, LUA_TTABLE);

    fluid_id_type fluid_id = FLUID_ID_NULL;

    if(!add_fluid(L, ctx, raw_name, def_idx, fluid_id)) {
        return lua_error(L);
    }

    lua_pushinteger(L, fluid_id);
    return 1;
}

void api::open_fluids_library(std::shared_ptr<lua_State>& lua, ModContext* ctx)
{
    auto L = lua.get();

    lua_newtable(L); // fluids

    lua_pushinteger(L, FLUID_GRAVITY_DOWN);
    lua_setfield(L, -2, "GRAVITY_DOWN");
    lua_pushinteger(L, FLUID_GRAVITY_UP);
    lua_setfield(L, -2, "GRAVITY_UP");

    lua_pushinteger(L, FLUID_ID_NULL);
    lua_setfield(L, -2, "NULL_FLUID");

    lua_pushlightuserdata(L, ctx);
    lua_pushcclosure(L, api_add, 1);
    lua_setfield(L, -2, "add");

    lua_setglobal(L, "fluids");
}
