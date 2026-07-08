#include "shared/pch.hh"

#include "shared/scripting/blocks_library.hh"

#include "core/utils/crc64.hh"

#include "core/identifier.hh"

#include "shared/utils/lua.hh"

#include "shared/block_registry.hh"
#include "shared/mod_context.hh"

class BlockDefReader final {
public:
    explicit BlockDefReader(lua_State* L, int def_idx, int proto_idx) noexcept;
    bool try_push(const char* key) const noexcept;

public:
    lua_State* L;
    int def_idx;
    int proto_idx;
};

BlockDefReader::BlockDefReader(lua_State* L, int def_idx, int proto_idx) noexcept : L(L), def_idx(def_idx), proto_idx(proto_idx)
{
    assert(L);
    assert(def_idx > 0);
    assert(proto_idx >= 0);
}

bool BlockDefReader::try_push(const char* key) const noexcept
{
    lua_getfield(L, def_idx, key);

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        if(proto_idx) {
            lua_getfield(L, proto_idx, key);

            if(lua_isnil(L, -1)) {
                lua_pop(L, 1);
                return false;
            }

            return true;
        }

        return false;
    }

    return true;
}

static std::optional<std::string_view> require_string(lua_State* L, int idx) noexcept
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

static std::optional<lua_Integer> require_integer(lua_State* L, int idx) noexcept
{
    int is_num = 0;
    auto value = lua_tointegerx(L, idx, &is_num);

    if(is_num) {
        return value;
    }

    lua_pushfstring(L, "expected an integer, got %s", lua_typename(L, lua_type(L, idx)));

    return std::nullopt;
}

static std::optional<lua_Integer> opt_integer(lua_State* L, int idx, lua_Integer default_value) noexcept
{
    if(lua_isnoneornil(L, idx))
        return default_value;
    return require_integer(L, idx);
}

static bool parse_drop_effects(lua_State* L, int when_idx, BlockDrop& drop, ModContext* ctx) noexcept
{
    lua_getfield(L, when_idx, "effects");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        return true;
    }

    auto effects_idx = lua_gettop(L);
    auto effects_count = lua_rawlen(L, effects_idx);

    drop.cond_effects.clear();
    drop.cond_effects.reserve(effects_count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(effects_count); ++i) {
        lua_rawgeti(L, effects_idx, i);

        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        drop.cond_effects.emplace_back(Identifier::from_string(value.value(), ctx->name_space()));

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return true;
}

static void parse_drop_tools(lua_State* L, int when_idx, BlockDrop& drop) noexcept
{
    lua_getfield(L, when_idx, "tools");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    drop.cond_tool_bits = static_cast<block_tool_bit>(utils::read_bitmask<unsigned>(L, lua_gettop(L)));

    lua_pop(L, 1);
}

static bool parse_drop_when(lua_State* L, int entry_idx, BlockDrop& drop, ModContext* ctx) noexcept
{
    lua_getfield(L, entry_idx, "when");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        return true;
    }

    auto when_idx = lua_gettop(L);

    if(!parse_drop_effects(L, when_idx, drop, ctx)) {
        return false;
    }

    parse_drop_tools(L, when_idx, drop);

    lua_pop(L, 1);

    return true;
}

static bool parse_drop_items(lua_State* L, int entry_idx, BlockDrop& drop, ModContext* ctx) noexcept
{
    lua_getfield(L, entry_idx, "items");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        return true;
    }

    auto items_idx = lua_gettop(L);
    auto items_count = lua_rawlen(L, items_idx);

    drop.items.clear();
    drop.items.reserve(items_count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(items_count); ++i) {
        lua_rawgeti(L, items_idx, i);

        auto item_idx = lua_gettop(L);

        BlockDropItem item {};

        lua_getfield(L, item_idx, "name");

        auto name = require_string(L, -1);

        if(!name.has_value()) {
            return false;
        }

        item.name = Identifier::from_string(name.value(), ctx->name_space());

        lua_pop(L, 1);

        lua_getfield(L, item_idx, "count");

        auto count = opt_integer(L, -1, 1);

        if(!count.has_value()) {
            return false;
        }

        item.count = static_cast<unsigned>(count.value());

        lua_pop(L, 1);

        drop.items.push_back(std::move(item));

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return true;
}

static bool parse_drops(lua_State* L, int drops_idx, ModContext* ctx, std::vector<BlockDrop>& out) noexcept
{
    auto count = lua_rawlen(L, drops_idx);

    out.clear();
    out.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, drops_idx, i);

        auto entry_idx = lua_gettop(L);

        BlockDrop drop {};
        drop.cond_tool_bits = BLOCK_TOOL_NONE;

        if(!parse_drop_when(L, entry_idx, drop, ctx)) {
            return false;
        }

        if(!parse_drop_items(L, entry_idx, drop, ctx)) {
            return false;
        }

        out.push_back(std::move(drop));

        lua_pop(L, 1);
    }

    return true;
}

static bool parse_overrides(lua_State* L, int idx, ModContext* ctx, BlockOverridePatch& patch) noexcept
{
    BlockDefReader reader(L, idx, 0);

    if(reader.try_push("render")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.render = static_cast<block_render>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("textures")) {
        emhash8::HashMap<std::string, std::vector<Identifier>> textures;
        auto textures_idx = lua_gettop(L);

        lua_pushnil(L);

        while(lua_next(L, textures_idx)) {
            auto slot_name = require_string(L, -2);

            if(!slot_name.has_value()) {
                return false;
            }

            auto slot = std::string(slot_name.value());
            auto count = lua_rawlen(L, -1);

            std::vector<Identifier> variants;
            variants.reserve(count);

            for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
                lua_rawgeti(L, -1, i);

                auto variant_name = require_string(L, -1);

                if(!variant_name.has_value()) {
                    return false;
                }

                variants.emplace_back(Identifier::from_string(variant_name.value(), ctx->name_space()));

                lua_pop(L, 1);
            }

            textures.insert_or_assign(std::move(slot), std::move(variants));

            lua_pop(L, 1);
        }

        patch.textures = std::move(textures);

        lua_pop(L, 1);
    }

    if(reader.try_push("animated")) {
        patch.animated = static_cast<bool>(lua_toboolean(L, -1));

        lua_pop(L, 1);
    }

    if(reader.try_push("model_name")) {
        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.model_name = Identifier::from_string(value.value(), ctx->name_space());

        lua_pop(L, 1);
    }

    if(reader.try_push("model_offset")) {
        patch.model_offset = utils::read_vector3f(L, lua_gettop(L)) / 16.0f;

        lua_pop(L, 1);
    }

    if(reader.try_push("bcoll_name")) {
        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.bcoll_name = Identifier::from_string(value.value(), ctx->name_space());

        lua_pop(L, 1);
    }

    if(reader.try_push("bcoll_offset")) {
        patch.bcoll_offset = utils::read_vector3f(L, lua_gettop(L)) / 16.0f;

        lua_pop(L, 1);
    }

    if(reader.try_push("health")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.health = static_cast<unsigned>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("sound")) {
        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.sound_set = Identifier::from_string(value.value(), ctx->name_space());

        lua_pop(L, 1);
    }

    if(reader.try_push("emission")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.emission = static_cast<block_light_type>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("dissipation")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.dissipation = static_cast<block_light_type>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("touch")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        patch.touch = static_cast<block_touch>(value.value());
        lua_pop(L, 1);
    }

    if(reader.try_push("touch_coeffs")) {
        patch.touch_coeffs = utils::read_vector3f(L, lua_gettop(L));
        lua_pop(L, 1);
    }

    if(reader.try_push("tags")) {
        patch.tags = static_cast<block_tag_bit>(utils::read_bitmask<unsigned>(L, lua_gettop(L)));
        lua_pop(L, 1);
    }

    if(reader.try_push("drops")) {
        std::vector<BlockDrop> drops;

        if(!parse_drops(L, lua_gettop(L), ctx, drops)) {
            return false;
        }

        patch.drops = std::move(drops);

        lua_pop(L, 1);
    }

    return true;
}

static bool parse_definition(const BlockDefReader& reader, ModContext* ctx, BlockDefinition& def) noexcept
{
    auto L = reader.L;

    if(reader.try_push("render")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.render = static_cast<block_render>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("model_name")) {
        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.model_name = Identifier::from_string(value.value(), ctx->name_space());

        lua_pop(L, 1);
    }

    if(reader.try_push("model_offset")) {
        def.model_offset = utils::read_vector3f(L, lua_gettop(L)) / 16.0f;

        lua_pop(L, 1);
    }

    if(reader.try_push("bcoll_name")) {
        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.bcoll_name = Identifier::from_string(value.value(), ctx->name_space());

        lua_pop(L, 1);
    }

    if(reader.try_push("bcoll_offset")) {
        def.bcoll_offset = utils::read_vector3f(L, lua_gettop(L)) / 16.0f;

        lua_pop(L, 1);
    }

    if(reader.try_push("animated")) {
        def.animated = static_cast<bool>(lua_toboolean(L, -1));

        lua_pop(L, 1);
    }

    if(reader.try_push("textures")) {
        auto textures_idx = lua_gettop(L);

        def.textures.clear();

        lua_pushnil(L);

        while(lua_next(L, textures_idx)) {
            auto slot_name = require_string(L, -2);

            if(!slot_name.has_value()) {
                return false;
            }

            auto slot = std::string(slot_name.value());
            auto count = lua_rawlen(L, -1);

            std::vector<Identifier> variants;
            variants.reserve(count);

            for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
                lua_rawgeti(L, -1, i);

                auto variant_name = require_string(L, -1);

                if(!variant_name.has_value()) {
                    return false;
                }

                variants.emplace_back(Identifier::from_string(variant_name.value(), ctx->name_space()));

                lua_pop(L, 1);
            }

            def.textures.insert_or_assign(std::move(slot), std::move(variants));

            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    }

    if(reader.try_push("health")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.health = static_cast<unsigned>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("sound")) {
        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.sound_set = Identifier::from_string(value.value(), ctx->name_space());

        lua_pop(L, 1);
    }

    if(reader.try_push("emission")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.emission = static_cast<block_light_type>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("dissipation")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.dissipation = static_cast<block_light_type>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("touch")) {
        auto value = require_integer(L, -1);

        if(!value.has_value()) {
            return false;
        }

        def.touch = static_cast<block_touch>(value.value());

        lua_pop(L, 1);
    }

    if(reader.try_push("touch_coeffs")) {
        def.touch_coeffs = utils::read_vector3f(L, lua_gettop(L));

        lua_pop(L, 1);
    }

    if(reader.try_push("tags")) {
        def.tags = static_cast<block_tag_bit>(utils::read_bitmask<unsigned>(L, lua_gettop(L)));

        lua_pop(L, 1);
    }

    if(reader.try_push("drops")) {
        if(!parse_drops(L, lua_gettop(L), ctx, def.drops)) {
            return false;
        }

        lua_pop(L, 1);
    }

    return true;
}

static bool parse_state_hint(lua_State* L, int entry_idx, BlockStateDecl& decl, BlockFamily& family) noexcept
{
    lua_getfield(L, entry_idx, "hint");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        return true;
    }

    auto hint_idx = lua_gettop(L);
    auto count = lua_rawlen(L, hint_idx);

    decl.hint.clear();
    decl.hint.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, hint_idx, i);

        auto value = require_string(L, -1);

        if(!value.has_value()) {
            return false;
        }

        decl.hint.push_back(family.state_hash(value.value()));

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return true;
}

static bool parse_states(lua_State* L, int idx, BlockFamily& family) noexcept
{
    lua_pushnil(L);

    while(lua_next(L, idx)) {
        auto state_name_str = require_string(L, -2);

        if(!state_name_str.has_value()) {
            return false;
        }

        auto state_name = std::string(state_name_str.value());
        auto entry_idx = lua_gettop(L);

        BlockStateDecl decl {};

        lua_getfield(L, entry_idx, "default");

        auto default_value = require_string(L, -1);

        if(!default_value.has_value()) {
            return false;
        }

        decl.default_value = family.state_hash(default_value.value());

        lua_pop(L, 1);

        if(!parse_state_hint(L, entry_idx, decl, family)) {
            return false;
        }

        auto key = static_cast<blockstate_key_type>(utils::crc64(state_name.data(), state_name.size()));
        family.states.insert_or_assign(key, std::move(decl));

        lua_pop(L, 1);
    }

    return true;
}

static bool parse_variant_when(lua_State* L, int entry_idx, BlockVariantRule& rule, BlockFamily& family) noexcept
{
    lua_getfield(L, entry_idx, "when");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        return true;
    }

    auto when_idx = lua_gettop(L);

    lua_pushnil(L);

    while(lua_next(L, when_idx)) {
        auto state_name_str = require_string(L, -2);

        if(!state_name_str.has_value()) {
            return false;
        }

        auto state_name = std::string(state_name_str.value());

        auto value_str = require_string(L, -1);

        if(!value_str.has_value()) {
            return false;
        }

        auto key = static_cast<blockstate_key_type>(utils::crc64(state_name.data(), state_name.size()));
        auto value = family.state_hash(value_str.value());

        rule.when.try_emplace(key, value);

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    return true;
}

static bool parse_variant_overrides(lua_State* L, int entry_idx, BlockVariantRule& rule, ModContext* ctx) noexcept
{
    lua_getfield(L, entry_idx, "overrides");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);

        return true;
    }

    if(!parse_overrides(L, lua_gettop(L), ctx, rule.overrides)) {
        return false;
    }

    lua_pop(L, 1);

    return true;
}

static bool parse_variants(lua_State* L, int idx, ModContext* ctx, BlockFamily& family) noexcept
{
    auto count = lua_rawlen(L, idx);

    family.variants.clear();
    family.variants.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, idx, i);

        auto entry_idx = lua_gettop(L);

        BlockVariantRule rule {};

        if(!parse_variant_when(L, entry_idx, rule, family)) {
            return false;
        }

        if(!parse_variant_overrides(L, entry_idx, rule, ctx)) {
            return false;
        }

        family.variants.push_back(std::move(rule));

        lua_pop(L, 1);
    }

    return true;
}

static BlockCallback parse_callback(lua_State* L, int table_idx, const char* key, ModContext* ctx) noexcept
{
    lua_getfield(L, table_idx, key);

    if(lua_isfunction(L, -1)) {
        BlockCallback callback {};
        callback.state = ctx->lua_state();
        callback.callback_ref = luaL_ref(L, LUA_REGISTRYINDEX); // pops the function
        return callback;
    }

    lua_pop(L, 1);
    return {};
}

static Identifier make_unique_id(const Identifier& id, const ModContext* ctx) noexcept
{
    auto is_free = [&](const Identifier& candidate) noexcept {
        return ctx->find_block(candidate) == BLOCK_ID_NULL && block_registry::find(candidate) == BLOCK_ID_NULL;
    };

    if(is_free(id)) {
        return id;
    }

    for(unsigned n = 1; n < 10000; ++n) {
        auto candidate = Identifier::from_parts(id.name_space(), std::format("{}~{}", id.value(), n));

        if(is_free(candidate)) {
            LOG_WARNING("blocks.add: {} is already registered, using {} instead", id.full_string(), candidate.full_string());
            return candidate;
        }
    }

    LOG_CRITICAL("blocks.add: {}: out of resolvable names", id.full_string());
    return id;
}

static int api_get(lua_State* L) noexcept
{
    auto ctx = static_cast<ModContext*>(lua_touserdata(L, lua_upvalueindex(1)));
    auto raw_name = luaL_checkstring(L, 1);
    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(!id.is_valid()) {
        lua_pushinteger(L, BLOCK_ID_NULL);
        return 1;
    }

    auto local_id = ctx->find_block(id);

    if(local_id == BLOCK_ID_NULL) {
        local_id = block_registry::find(id);
    }

    lua_pushinteger(L, local_id);
    return 1;
}

static int api_has_tag(lua_State* L) noexcept
{
    auto id = static_cast<block_id_type>(luaL_checkinteger(L, 1));
    auto tag = static_cast<block_tag_bit>(luaL_checkinteger(L, 2));

    lua_pushboolean(L, block_registry::has_tag_any(id, tag));
    return 1;
}

static bool add_block(lua_State* L, ModContext* ctx, const char* raw_name, int def_idx, int proto_idx, block_id_type& out_block_id) noexcept
{
    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(!id.is_valid()) {
        lua_pushfstring(L, "blocks.add: malformed name: %s", raw_name);

        return false;
    }

    id = make_unique_id(id, ctx);

    BlockDefinition def {};
    def.render = BLOCK_RENDER_NONE;
    def.animated = false;
    def.health = 0;
    def.tools = BLOCK_TOOL_NONE;
    def.emission = BLOCK_LIGHT_MIN;
    def.dissipation = BLOCK_LIGHT_MIN;
    def.touch = BLOCK_TOUCH_SOLID;
    def.touch_coeffs = Eigen::Vector3f::Ones();
    def.tags = static_cast<block_tag_bit>(0);
    def.family = BLOCK_FAMILY_ID_NULL;

    BlockDefReader reader(L, def_idx, proto_idx);

    if(!parse_definition(reader, ctx, def)) {
        return false;
    }

    // a block that resolves into variants is allowed to omit model_name at
    // the top level - each variants[].overrides is expected to supply its
    // own (eg. a slab's base registration has no sensible single model)
    auto has_variants = reader.try_push("variants");

    if(has_variants) {
        lua_pop(L, 1);
    }

    if(!has_variants && def.render != BLOCK_RENDER_NONE && def.model_name.is_empty()) {
        lua_pushfstring(L, "blocks.add: [%s]: model_name is required unless render is blocks.RENDER_NONE", id.full_string().data());
        return false;
    }

    auto block_id = ctx->register_block(id, def);
    auto has_checked_key = has_variants;

    for(const auto& key : std::array { "states", "variants", "on_rtick", "on_stick", "on_place", "on_break", "on_interact" }) {
        if(reader.try_push(key)) {
            has_checked_key = true;
            lua_pop(L, 1);
            break;
        }
    }

    if(has_checked_key) {
        BlockFamily family {};
        family.name = id;
        family.base_id = block_id;

        if(reader.try_push("states")) {
            auto states_ok = parse_states(L, lua_gettop(L), family);
            lua_pop(L, 1);

            if(!states_ok) {
                return false;
            }
        }

        if(reader.try_push("variants")) {
            auto variants_ok = parse_variants(L, lua_gettop(L), ctx, family);
            lua_pop(L, 1);

            if(!variants_ok) {
                return false;
            }
        }

        family.on_rtick = parse_callback(L, def_idx, "on_rtick", ctx);
        family.on_stick = parse_callback(L, def_idx, "on_stick", ctx);
        family.on_place = parse_callback(L, def_idx, "on_place", ctx);
        family.on_break = parse_callback(L, def_idx, "on_break", ctx);
        family.on_interact = parse_callback(L, def_idx, "on_interact", ctx);

        auto family_id = ctx->register_block_family(std::move(family));
        ctx->set_block_family(block_id, family_id);
    }

    out_block_id = block_id;
    return true;
}

static int api_add(lua_State* L) noexcept
{
    auto ctx = static_cast<ModContext*>(lua_touserdata(L, lua_upvalueindex(1)));

    auto argc = lua_gettop(L);
    auto raw_name = luaL_checkstring(L, 1);

    int proto_idx = 0;
    int def_idx = 2;

    if(argc >= 3) {
        proto_idx = 2;
        def_idx = 3;

        luaL_checktype(L, proto_idx, LUA_TTABLE);
    }

    luaL_checktype(L, def_idx, LUA_TTABLE);

    block_id_type block_id = BLOCK_ID_NULL;

    if(!add_block(L, ctx, raw_name, def_idx, proto_idx, block_id)) {
        return lua_error(L);
    }

    lua_pushinteger(L, block_id);
    return 1;
}

void scripting::open_blocks_library(std::shared_ptr<lua_State>& lua, ModContext* ctx) noexcept
{
    assert(lua);
    assert(ctx);

    auto L = lua.get();

    lua_newtable(L); // blocks

    lua_pushinteger(L, BLOCK_RENDER_SOLID);
    lua_setfield(L, -2, "RENDER_SOLID");
    lua_pushinteger(L, BLOCK_RENDER_ALPHA);
    lua_setfield(L, -2, "RENDER_ALPHA");
    lua_pushinteger(L, BLOCK_RENDER_NONE);
    lua_setfield(L, -2, "RENDER_NONE");

    lua_pushinteger(L, BLOCK_FACE_NORTH);
    lua_setfield(L, -2, "FACE_NORTH");
    lua_pushinteger(L, BLOCK_FACE_SOUTH);
    lua_setfield(L, -2, "FACE_SOUTH");
    lua_pushinteger(L, BLOCK_FACE_EAST);
    lua_setfield(L, -2, "FACE_EAST");
    lua_pushinteger(L, BLOCK_FACE_WEST);
    lua_setfield(L, -2, "FACE_WEST");
    lua_pushinteger(L, BLOCK_FACE_TOP);
    lua_setfield(L, -2, "FACE_TOP");
    lua_pushinteger(L, BLOCK_FACE_BOTTOM);
    lua_setfield(L, -2, "FACE_BOTTOM");
    lua_pushinteger(L, BLOCK_FACE_TOP);
    lua_setfield(L, -2, "FACE_UP");
    lua_pushinteger(L, BLOCK_FACE_BOTTOM);
    lua_setfield(L, -2, "FACE_DOWN");

    lua_pushinteger(L, BLOCK_TOOL_NONE);
    lua_setfield(L, -2, "TOOL_NONE");
    lua_pushinteger(L, BLOCK_TOOL_BLADE);
    lua_setfield(L, -2, "TOOL_BLADE");
    lua_pushinteger(L, BLOCK_TOOL_XBLADE);
    lua_setfield(L, -2, "TOOL_XBLADE");
    lua_pushinteger(L, BLOCK_TOOL_AXE);
    lua_setfield(L, -2, "TOOL_AXE");
    lua_pushinteger(L, BLOCK_TOOL_HOE);
    lua_setfield(L, -2, "TOOL_HOE");
    lua_pushinteger(L, BLOCK_TOOL_SHOVEL);
    lua_setfield(L, -2, "TOOL_SHOVEL");
    lua_pushinteger(L, BLOCK_TOOL_HAMMER);
    lua_setfield(L, -2, "TOOL_HAMMER");
    lua_pushinteger(L, BLOCK_TOOL_PICK);
    lua_setfield(L, -2, "TOOL_PICK");

    lua_pushinteger(L, BLOCK_TOUCH_NONE);
    lua_setfield(L, -2, "TOUCH_NONE");
    lua_pushinteger(L, BLOCK_TOUCH_SOLID);
    lua_setfield(L, -2, "TOUCH_SOLID");
    lua_pushinteger(L, BLOCK_TOUCH_BOUNCE);
    lua_setfield(L, -2, "TOUCH_BOUNCE");
    lua_pushinteger(L, BLOCK_TOUCH_THROUGH);
    lua_setfield(L, -2, "TOUCH_THROUGH");

    lua_pushinteger(L, BLOCK_TAG_GAS);
    lua_setfield(L, -2, "TAG_GAS");
    lua_pushinteger(L, BLOCK_TAG_ROCK);
    lua_setfield(L, -2, "TAG_ROCK");
    lua_pushinteger(L, BLOCK_TAG_SOIL);
    lua_setfield(L, -2, "TAG_SOIL");
    lua_pushinteger(L, BLOCK_TAG_TURF);
    lua_setfield(L, -2, "TAG_TURF");
    lua_pushinteger(L, BLOCK_TAG_FOIL);
    lua_setfield(L, -2, "TAG_FOIL");
    lua_pushinteger(L, BLOCK_TAG_WOOD);
    lua_setfield(L, -2, "TAG_WOOD");

    lua_pushinteger(L, BLOCK_ID_NULL);
    lua_setfield(L, -2, "NULL_BLOCK");

    lua_pushlightuserdata(L, ctx);
    lua_pushcclosure(L, &api_get, 1);
    lua_setfield(L, -2, "get");

    lua_pushcfunction(L, &api_has_tag);
    lua_setfield(L, -2, "has_tag");

    lua_pushlightuserdata(L, ctx);
    lua_pushcclosure(L, &api_add, 1);
    lua_setfield(L, -2, "add");

    lua_setglobal(L, "blocks");
}
