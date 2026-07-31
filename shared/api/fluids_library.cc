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

    lua_getfield(L, def_idx, "full_level");

    auto full_level = utils::require_integer(L, -1);

    if(!full_level.has_value()) {
        return false;
    }

    def.full_level = static_cast<unsigned>(full_level.value());
    def.full_level = std::clamp(def.full_level, 1U, 15U);
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

    lua_getfield(L, def_idx, "fog_density");
    auto fog_density = utils::opt_number(L, -1, 1.0);

    if(!fog_density.has_value()) {
        return false;
    }

    def.fog_density = std::max(static_cast<float>(fog_density.value()), 1.0f);
    lua_pop(L, 1);

    lua_getfield(L, def_idx, "fog_color");

    if(!lua_isnil(L, -1)) {
        auto fog_color = utils::read_vector<float, 3>(L, -1);

        if(!fog_color.has_value()) {
            return false;
        }

        def.fog_color = fog_color.value();
        def.fog_color = def.fog_color.cwiseMax(0.0f).cwiseMin(1.0f);
    }

    lua_pop(L, 1);

    lua_getfield(L, def_idx, "tint");
    auto tint_name = utils::opt_string(L, -1, {});

    if(!tint_name.has_value()) {
        return false;
    }

    if(tint_name->size()) {
        def.tint_name = Identifier::from_string(tint_name.value(), ctx->name_space());
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

    lua_getfield(L, -1, "still_mask");

    if(!lua_isnil(L, -1)) {
        auto still_mask = utils::require_string(L, -1);

        if(!still_mask.has_value()) {
            return false;
        }

        def.still_mask = Identifier::from_string(still_mask.value(), ctx->name_space());

        if(!def.still_mask->is_valid()) {
            lua_pushfstring(L, "malformed still mask name: %s", still_mask.value());
            return false;
        }
    }

    lua_pop(L, 1);

    lua_getfield(L, -1, "flowing_mask");

    if(!lua_isnil(L, -1)) {
        auto flowing_mask = utils::require_string(L, -1);

        if(!flowing_mask.has_value()) {
            return false;
        }

        def.flowing_mask = Identifier::from_string(flowing_mask.value(), ctx->name_space());

        if(!def.flowing_mask->is_valid()) {
            lua_pushfstring(L, "malformed flowing mask name: %s", flowing_mask.value());
            return false;
        }
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

    def.gravity = FLUID_GRAVITY_DOWN;
    def.opaque = false;

    def.fog_density = 1.0f;
    def.fog_color.setOnes();

    def.tint_name = {};
    def.tint = TINT_ID_NULL;

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
