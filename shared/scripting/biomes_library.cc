#include "shared/pch.hh"

#include "shared/scripting/biomes_library.hh"

#include "core/identifier.hh"

#include "shared/biome.hh"
#include "shared/biome_registry.hh"
#include "shared/block_registry.hh"
#include "shared/mod_context.hh"
#include "shared/utils/lua.hh"

static block_id_type resolve_block(lua_State* L, ModContext* ctx, std::string_view raw_name)
{
    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(id.is_valid()) {
        auto local_id = ctx->find_block(id);

        if(local_id) {
            return local_id;
        }
        else {
            return block_registry::find(id);
        }
    }

    return BLOCK_ID_NULL;
}

static std::optional<BiomeBlockPalette> parse_block_palette(lua_State* L, int idx, ModContext* ctx)
{
    BiomeBlockPalette palette {};

    const std::array fields = {
        std::make_pair("base", &palette.base),
        std::make_pair("filler", &palette.filler),
        std::make_pair("surface", &palette.surface),
        std::make_pair("fluid", &palette.fluid),
        std::make_pair("ceiling", &palette.ceiling),
    };

    for(auto& it : fields) {
        lua_getfield(L, idx, it.first);

        if(lua_isnil(L, -1)) {
            lua_pop(L, 1);
            continue;
        }

        auto name = utils::require_string(L, -1);

        if(!name.has_value()) {
            return std::nullopt;
        }

        auto block_id = resolve_block(L, ctx, name.value());
        it.second[0] = block_id;
        lua_pop(L, 1);
    }

    return palette;
}

static std::optional<BiomeStratum> parse_stratum(lua_State* L, int idx, ModContext* ctx)
{
    BiomeStratum stratum {};

    lua_getfield(L, idx, "depth");
    auto depth = utils::require_integer(L, -1);

    if(!depth.has_value()) {
        return std::nullopt;
    }

    stratum.depth = static_cast<int>(depth.value());
    lua_pop(L, 1);

    lua_getfield(L, idx, "block");
    auto block_name = utils::require_string(L, -1);

    if(!block_name.has_value()) {
        return std::nullopt;
    }

    stratum.block = resolve_block(L, ctx, block_name.value());
    lua_pop(L, 1);

    if(stratum.block == BLOCK_ID_NULL) {
        return std::nullopt;
    }

    return stratum;
}

static std::optional<std::vector<BiomeStratum>> parse_strata(lua_State* L, int idx, ModContext* ctx)
{
    auto count = lua_rawlen(L, idx);

    std::vector<BiomeStratum> strata {};
    strata.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, idx, i);

        auto entry_idx = lua_gettop(L);
        auto stratum = parse_stratum(L, entry_idx, ctx);

        if(!stratum.has_value()) {
            return std::nullopt;
        }

        strata.push_back(std::move(stratum.value()));

        lua_pop(L, 1);
    }

    return strata;
}

static std::optional<BiomeFeatureEntry> parse_feature_entry(lua_State* L, int idx, ModContext* ctx)
{
    BiomeFeatureEntry entry {};

    lua_getfield(L, idx, "id");
    auto id = utils::require_string(L, -1);

    if(!id.has_value()) {
        return std::nullopt;
    }

    entry.feature_id = Identifier::from_string(id.value(), ctx->name_space());
    lua_pop(L, 1);

    lua_getfield(L, idx, "chance");
    auto chance = utils::opt_number(L, -1, 0.5);

    if(!chance.has_value()) {
        return std::nullopt;
    }

    entry.chance = static_cast<float>(chance.value());
    lua_pop(L, 1);

    lua_getfield(L, idx, "requires_sky");
    entry.requires_sky = static_cast<bool>(lua_toboolean(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, idx, "requires_floor");
    entry.requires_floor = static_cast<bool>(lua_toboolean(L, -1));
    lua_pop(L, 1);

    const std::array fields = {
        std::make_pair("min_depth", &entry.min_depth),
        std::make_pair("max_depth", &entry.max_depth),
        std::make_pair("min_altitude", &entry.min_altitude),
        std::make_pair("max_altitude", &entry.max_altitude),
    };

    for(auto& it : fields) {
        lua_getfield(L, idx, it.first);

        if(!lua_isnil(L, -1)) {
            auto value = utils::require_integer(L, -1);

            if(!value.has_value()) {
                return std::nullopt;
            }

            it.second[0] = static_cast<int>(value.value());
        }

        lua_pop(L, 1);
    }

    return entry;
}

static std::optional<std::vector<BiomeFeatureEntry>> parse_features(lua_State* L, int idx, ModContext* ctx)
{
    auto count = lua_rawlen(L, idx);

    std::vector<BiomeFeatureEntry> features {};
    features.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, idx, i);

        auto entry_idx = lua_gettop(L);
        auto feature = parse_feature_entry(L, entry_idx, ctx);

        if(!feature.has_value()) {
            return std::nullopt;
        }

        lua_pop(L, 1);

        features.push_back(std::move(feature.value()));
    }

    return features;
}

static std::optional<BiomeDefinition> parse_definition(lua_State* L, int idx, ModContext* ctx)
{
    BiomeDefinition def {};
    def.name = Identifier::from_string(luaL_checkstring(L, 1), ctx->name_space());

    lua_getfield(L, idx, "realm");
    auto realm = utils::require_integer(L, -1);

    if(!realm.has_value()) {
        return std::nullopt;
    }

    def.realm = static_cast<biome_realm>(realm.value());
    lua_pop(L, 1);

    lua_getfield(L, idx, "temperature");
    auto temperature = utils::require_integer(L, -1);

    if(!temperature.has_value()) {
        return std::nullopt;
    }

    def.temperature = static_cast<int>(temperature.value());
    lua_pop(L, 1);

    lua_getfield(L, idx, "humidity");
    auto humidity = utils::require_integer(L, -1);

    if(!humidity.has_value()) {
        return std::nullopt;
    }

    def.humidity = static_cast<int>(humidity.value());
    lua_pop(L, 1);

    switch(def.realm) {
        case BIOME_REALM_SURFACE:
            lua_getfield(L, idx, "weirdness");
            break;

        case BIOME_REALM_CAVE:
        case BIOME_REALM_DEEP:
            lua_getfield(L, idx, "depth");
            break;

        case BIOME_REALM_SKY:
            lua_getfield(L, idx, "altitude");
            break;

        default:
            lua_pushfstring(L, "biomes.add: invalid realm: %d", static_cast<int>(def.realm));
            return std::nullopt;
    }

    auto extra_axis = utils::require_integer(L, -1);

    if(!extra_axis.has_value()) {
        return std::nullopt;
    }

    def.extra_axis = static_cast<int>(extra_axis.value());
    lua_pop(L, 1);

    lua_getfield(L, idx, "priority");

    auto priority = utils::opt_integer(L, -1, 0);

    if(!priority.has_value()) {
        return std::nullopt;
    }

    def.priority = static_cast<int>(priority.value());
    lua_pop(L, 1);

    lua_getfield(L, idx, "blocks");

    if(!lua_isnil(L, -1)) {
        auto blocks = parse_block_palette(L, lua_gettop(L), ctx);

        if(!blocks.has_value()) {
            return std::nullopt;
        }

        def.blocks = std::move(blocks.value());
    }

    lua_pop(L, 1);

    lua_getfield(L, idx, "strata");

    if(!lua_isnil(L, -1)) {
        auto strata = parse_strata(L, lua_gettop(L), ctx);

        if(!strata.has_value()) {
            return std::nullopt;
        }

        def.strata = std::move(strata.value());
    }

    lua_pop(L, 1);

    lua_getfield(L, idx, "features");

    if(!lua_isnil(L, -1)) {
        auto features = parse_features(L, lua_gettop(L), ctx);

        if(!features.has_value()) {
            return std::nullopt;
        }

        def.features = std::move(features.value());
    }

    lua_pop(L, 1);

    return def;
}

static int api_add(lua_State* L)
{
    auto ctx = static_cast<ModContext*>(lua_touserdata(L, lua_upvalueindex(1)));

    auto raw_name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);

    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(!id.is_valid()) {
        lua_pushfstring(L, "biomes.add: malformed name: %s", raw_name);
        return lua_error(L);
    }

    auto def = parse_definition(L, 2, ctx);

    if(!def.has_value()) {
        return lua_error(L);
    }

    auto biome_id = ctx->register_biome(id, std::move(def.value()));
    lua_pushinteger(L, biome_id);
    return 1;
}

void scripting::open_biomes_library(std::shared_ptr<lua_State>& lua, ModContext* ctx)
{
    assert(lua);
    assert(ctx);

    auto L = lua.get();

    lua_newtable(L); // biomes

    lua_pushinteger(L, BIOME_TEMP_MIN);
    lua_setfield(L, -2, "TEMP_MIN");
    lua_pushinteger(L, BIOME_TEMP_MAX);
    lua_setfield(L, -2, "TEMP_MAX");
    lua_pushinteger(L, BIOME_HUMID_MIN);
    lua_setfield(L, -2, "HUMID_MIN");
    lua_pushinteger(L, BIOME_HUMID_MAX);
    lua_setfield(L, -2, "HUMID_MAX");
    lua_pushinteger(L, BIOME_AXIS_MIN);
    lua_setfield(L, -2, "AXIS_MIN");
    lua_pushinteger(L, BIOME_AXIS_MAX);
    lua_setfield(L, -2, "AXIS_MAX");

    lua_pushinteger(L, BIOME_REALM_SURFACE);
    lua_setfield(L, -2, "REALM_SURFACE");
    lua_pushinteger(L, BIOME_REALM_CAVE);
    lua_setfield(L, -2, "REALM_CAVE");
    lua_pushinteger(L, BIOME_REALM_DEEP);
    lua_setfield(L, -2, "REALM_DEEP");
    lua_pushinteger(L, BIOME_REALM_SKY);
    lua_setfield(L, -2, "REALM_SKY");

    lua_pushlightuserdata(L, ctx);
    lua_pushcclosure(L, &api_add, 1);
    lua_setfield(L, -2, "add");

    lua_setglobal(L, "biomes");
}
