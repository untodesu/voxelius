#include "shared/pch.hh"

#include "shared/api/biomes_library.hh"

#include "core/identifier.hh"

#include "shared/mod_context.hh"
#include "shared/utils/lua.hh"
#include "shared/world/biome_registry.hh"

static Identifier make_unique_id(const Identifier& id, const ModContext* ctx)
{
    auto is_free = [&](const Identifier& candidate) {
        return ctx->find_biome(candidate) == BIOME_ID_NULL && biome_registry::find(candidate) == BIOME_ID_NULL;
    };

    if(is_free(id)) {
        return id;
    }

    for(unsigned n = 1; n < 10000; ++n) {
        auto candidate = Identifier::from_parts(id.name_space(), std::format("{}~{}", id.value(), n));

        if(is_free(candidate)) {
            LOG_WARNING("biomes.add: {} is already registered, using {} instead", id.full_string(), candidate.full_string());
            return candidate;
        }
    }

    LOG_CRITICAL("biomes.add: {}: out of resolvable names", id.full_string());
    return id;
}

static bool parse_tag_bitmask(lua_State* L, int idx, block_tag_bit& out)
{
    if(lua_isnil(L, idx)) {
        out = static_cast<block_tag_bit>(0);
        return true;
    }

    if(!lua_istable(L, idx)) {
        lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, idx)));
        return false;
    }

    out = static_cast<block_tag_bit>(utils::read_bitmask<unsigned>(L, idx));
    return true;
}

static bool parse_scatter_entry(lua_State* L, int entry_idx, BiomeScatterEntry& entry, ModContext* ctx)
{
    lua_getfield(L, entry_idx, "feature");
    auto feature = utils::require_string(L, -1);

    if(!feature.has_value()) {
        return false;
    }

    entry.feature = Identifier::from_string(feature.value(), ctx->name_space());

    lua_pop(L, 1);

    lua_getfield(L, entry_idx, "chance");
    auto chance = utils::opt_number(L, -1, 0.5);

    if(!chance.has_value()) {
        return false;
    }

    entry.chance = static_cast<float>(chance.value());

    lua_pop(L, 1);

    lua_getfield(L, entry_idx, "need_above");

    if(!parse_tag_bitmask(L, lua_gettop(L), entry.need_above)) {
        return false;
    }

    lua_pop(L, 1);

    lua_getfield(L, entry_idx, "need_below");

    if(!parse_tag_bitmask(L, lua_gettop(L), entry.need_below)) {
        return false;
    }

    lua_pop(L, 1);

    return true;
}

static bool parse_palette_entry(lua_State* L, int entry_idx, BiomePaletteEntry& entry, ModContext* ctx)
{
    lua_getfield(L, entry_idx, "name");
    auto name = utils::require_string(L, -1);

    if(!name.has_value()) {
        return false;
    }

    entry.name = Identifier::from_string(name.value(), ctx->name_space());
    lua_pop(L, 1);

    lua_getfield(L, entry_idx, "states");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return true;
    }

    if(!lua_istable(L, -1)) {
        lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, -1)));
        return false;
    }

    entry.states.clear();

    lua_pushnil(L);

    while(lua_next(L, -2)) {
        auto key_str = utils::require_string(L, -2);

        if(!key_str.has_value()) {
            return false;
        }

        auto value_str = utils::require_string(L, -1);

        if(!value_str.has_value()) {
            return false;
        }

        entry.states.try_emplace(std::string(key_str.value()), std::string(value_str.value()));

        lua_pop(L, 1);
    }

    entry.cached = BIOME_ID_NULL;

    lua_pop(L, 1);

    return true;
}

static bool parse_palette(lua_State* L, int palette_idx, BiomeDefinition& def, ModContext* ctx)
{
    const std::array palette_fields = {
        std::make_pair("basic", &def.palette_basic),
        std::make_pair("filler", &def.palette_filler),
        std::make_pair("surface", &def.palette_surface),
        std::make_pair("fluid", &def.palette_fluid),
    };

    for(const auto& [field_name, field_ptr] : palette_fields) {
        lua_getfield(L, palette_idx, field_name);

        if(!lua_isnil(L, -1)) {
            if(!parse_palette_entry(L, lua_gettop(L), field_ptr[0], ctx)) {
                return false;
            }
        }

        lua_pop(L, 1);
    }

    return true;
}

static bool parse_definition(lua_State* L, int def_idx, BiomeDefinition& def, ModContext* ctx)
{
    lua_getfield(L, def_idx, "realm");
    auto realm = utils::require_integer(L, -1);

    if(!realm.has_value()) {
        return false;
    }

    def.realm = static_cast<biome_realm>(realm.value());
    lua_pop(L, 1);

    const std::array lut_fields = {
        std::make_pair("lut_temp", &def.lut_temp),
        std::make_pair("lut_humd", &def.lut_humd),
        std::make_pair("lut_axis", &def.lut_axis),
    };

    for(const auto& [field_name, field_ptr] : lut_fields) {
        lua_getfield(L, def_idx, field_name);

        if(!lua_isnil(L, -1)) {
            auto value = utils::require_integer(L, -1);

            if(!value.has_value()) {
                return false;
            }

            if(value.value() < 0 || value.value() > 99) {
                lua_pushliteral(L, "lut value out of range [0..99]");
                return false;
            }

            field_ptr[0] = static_cast<std::uint8_t>(value.value());
        }

        lua_pop(L, 1);
    }

    lua_getfield(L, def_idx, "priority");
    auto priority = utils::opt_integer(L, -1, 0);

    if(!priority.has_value()) {
        return false;
    }

    if(priority.value() < 0) {
        lua_pushliteral(L, "priority must be non-negative");
        return false;
    }

    def.priority = static_cast<unsigned>(priority.value());
    lua_pop(L, 1);

    lua_getfield(L, def_idx, "palette");

    if(!lua_isnil(L, -1)) {
        if(!lua_istable(L, -1)) {
            lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, -1)));
            return false;
        }

        if(!parse_palette(L, lua_gettop(L), def, ctx)) {
            return false;
        }
    }

    lua_pop(L, 1);

    lua_getfield(L, def_idx, "scatter");

    def.scatter.clear();

    if(!lua_isnil(L, -1)) {
        if(!lua_istable(L, -1)) {
            lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, -1)));
            return false;
        }

        def.scatter.reserve(lua_rawlen(L, -1));

        lua_pushnil(L);

        while(lua_next(L, -2)) {
            auto entry_idx = lua_gettop(L);

            if(!lua_istable(L, entry_idx)) {
                lua_pushfstring(L, "expected a table, got %s", lua_typename(L, lua_type(L, entry_idx)));
                return false;
            }

            BiomeScatterEntry entry {};

            if(!parse_scatter_entry(L, entry_idx, entry, ctx)) {
                return false;
            }

            def.scatter.push_back(std::move(entry));

            lua_pop(L, 1);
        }
    }

    lua_pop(L, 1);

    return true;
}

static bool add_biome(lua_State* L, ModContext* ctx, const char* raw_name, int def_idx, biome_id_type& out_biome_id)
{
    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(!id.is_valid()) {
        lua_pushfstring(L, "biomes.add: malformed name: %s", raw_name);
        return false;
    }

    id = make_unique_id(id, ctx);

    BiomeDefinition def {};

    def.lut_temp = 50;
    def.lut_humd = 50;
    def.lut_axis = 50;

    def.priority = 0;

    if(!parse_definition(L, def_idx, def, ctx)) {
        return false;
    }

    out_biome_id = ctx->register_biome(std::move(id), std::move(def));

    return true;
}

static int api_add(lua_State* L)
{
    auto ctx = static_cast<ModContext*>(lua_touserdata(L, lua_upvalueindex(1)));

    auto raw_name = luaL_checkstring(L, 1);

    int def_idx = 2;

    luaL_checktype(L, def_idx, LUA_TTABLE);

    biome_id_type biome_id = BIOME_ID_NULL;

    if(!add_biome(L, ctx, raw_name, def_idx, biome_id)) {
        return lua_error(L);
    }

    lua_pushinteger(L, biome_id);
    return 1;
}

void api::open_biomes_library(std::shared_ptr<lua_State>& lua, ModContext* ctx)
{
    assert(lua);
    assert(ctx);

    auto L = lua.get();

    lua_newtable(L); // biomes

    lua_pushinteger(L, BIOME_REALM_SURFACE);
    lua_setfield(L, -2, "REALM_SURFACE");
    lua_pushinteger(L, BIOME_REALM_UNDERGROUND);
    lua_setfield(L, -2, "REALM_UNDERGROUND");
    lua_pushinteger(L, BIOME_REALM_THE_DEPTHS);
    lua_setfield(L, -2, "REALM_THE_DEPTHS");
    lua_pushinteger(L, BIOME_REALM_SKY);
    lua_setfield(L, -2, "REALM_SKY");

    lua_pushinteger(L, BIOME_ID_NULL);
    lua_setfield(L, -2, "NULL_BIOME");

    lua_pushlightuserdata(L, ctx);
    lua_pushcclosure(L, &api_add, 1);
    lua_setfield(L, -2, "add");

    lua_setglobal(L, "biomes");
}
