#include "shared/pch.hh"

#include "shared/scripting/blocks_api.hh"

#include "core/utils/crc64.hh"

#include "core/identifier.hh"

#include "shared/utils/lua.hh"

#include "shared/block_registry.hh"
#include "shared/mod_context.hh"

class FieldHelper final {
public:
    explicit FieldHelper(lua_State* L, int def_idx, int proto_idx) noexcept;
    bool try_push(const char* key) const noexcept;

public:
    lua_State* L;
    int def_idx;
    int proto_idx;
};

FieldHelper::FieldHelper(lua_State* L, int def_idx, int proto_idx) noexcept : L(L), def_idx(def_idx), proto_idx(proto_idx)
{
    assert(L);
    assert(def_idx > 0);
    assert(proto_idx >= 0);
}

bool FieldHelper::try_push(const char* key) const noexcept
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

static void parse_drop_effects(lua_State* L, int when_idx, BlockDrop& drop, ModContext* ctx) noexcept
{
    lua_getfield(L, when_idx, "effects");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    auto effects_idx = lua_gettop(L);
    auto effects_count = lua_rawlen(L, effects_idx);

    drop.cond_effects.clear();
    drop.cond_effects.reserve(effects_count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(effects_count); ++i) {
        lua_rawgeti(L, effects_idx, i);
        drop.cond_effects.emplace_back(Identifier::from_string(luaL_checkstring(L, -1), ctx->name_space()));
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
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

static void parse_drop_when(lua_State* L, int entry_idx, BlockDrop& drop, ModContext* ctx) noexcept
{
    lua_getfield(L, entry_idx, "when");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    auto when_idx = lua_gettop(L);

    parse_drop_effects(L, when_idx, drop, ctx);
    parse_drop_tools(L, when_idx, drop);

    lua_pop(L, 1);
}

static void parse_drop_items(lua_State* L, int entry_idx, BlockDrop& drop, ModContext* ctx) noexcept
{
    lua_getfield(L, entry_idx, "items");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
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
        item.name = Identifier::from_string(luaL_checkstring(L, -1), ctx->name_space());
        lua_pop(L, 1);

        lua_getfield(L, item_idx, "count");
        item.count = static_cast<unsigned>(luaL_optinteger(L, -1, 1));
        lua_pop(L, 1);

        drop.items.push_back(std::move(item));

        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

static std::vector<BlockDrop> parse_drops(lua_State* L, int drops_idx, ModContext* ctx) noexcept
{
    auto count = lua_rawlen(L, drops_idx);

    std::vector<BlockDrop> drops;
    drops.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, drops_idx, i);

        auto entry_idx = lua_gettop(L);

        BlockDrop drop {};
        drop.cond_tool_bits = BLOCK_TOOL_NONE;

        parse_drop_when(L, entry_idx, drop, ctx);
        parse_drop_items(L, entry_idx, drop, ctx);

        drops.push_back(std::move(drop));

        lua_pop(L, 1);
    }

    return drops;
}

static BlockOverridePatch parse_overrides(lua_State* L, int idx, ModContext* ctx) noexcept
{
    BlockOverridePatch patch {};
    FieldHelper helper(L, idx, 0);

    if(helper.try_push("render")) {
        patch.render = static_cast<block_render>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }

    if(helper.try_push("textures")) {
        std::unordered_map<std::string, std::vector<Identifier>> textures;
        auto textures_idx = lua_gettop(L);

        lua_pushnil(L);

        while(lua_next(L, textures_idx)) {
            auto slot = std::string(luaL_checkstring(L, -2));
            auto count = lua_rawlen(L, -1);

            std::vector<Identifier> variants;
            variants.reserve(count);

            for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
                lua_rawgeti(L, -1, i);
                variants.emplace_back(Identifier::from_string(luaL_checkstring(L, -1), ctx->name_space()));
                lua_pop(L, 1);
            }

            textures.insert_or_assign(std::move(slot), std::move(variants));

            lua_pop(L, 1);
        }

        patch.textures = std::move(textures);

        lua_pop(L, 1);
    }

    if(helper.try_push("animated")) {
        patch.animated = static_cast<bool>(lua_toboolean(L, -1));
        lua_pop(L, 1);
    }

    if(helper.try_push("model_name")) {
        patch.model_name = Identifier::from_string(luaL_checkstring(L, -1), ctx->name_space());
        lua_pop(L, 1);
    }

    if(helper.try_push("model_offset")) {
        patch.model_offset = utils::read_vector3f(L, lua_gettop(L)) / 16.0f;
        lua_pop(L, 1);
    }

    if(helper.try_push("bcoll_name")) {
        patch.bcoll_name = Identifier::from_string(luaL_checkstring(L, -1), ctx->name_space());
        lua_pop(L, 1);
    }

    if(helper.try_push("bcoll_offset")) {
        patch.bcoll_offset = utils::read_vector3f(L, lua_gettop(L)) / 16.0f;
        lua_pop(L, 1);
    }

    if(helper.try_push("health")) {
        patch.health = static_cast<unsigned>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }

    if(helper.try_push("sound")) {
        patch.sound_set = Identifier::from_string(luaL_checkstring(L, -1), ctx->name_space());
        lua_pop(L, 1);
    }

    if(helper.try_push("emission")) {
        patch.emission = static_cast<block_light_type>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }

    if(helper.try_push("dissipation")) {
        patch.dissipation = static_cast<block_light_type>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }

    if(helper.try_push("touch")) {
        patch.touch = static_cast<block_touch>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }

    if(helper.try_push("touch_coeffs")) {
        patch.touch_coeffs = utils::read_vector3f(L, lua_gettop(L));
        lua_pop(L, 1);
    }

    if(helper.try_push("tags")) {
        patch.tags = static_cast<block_tag_bit>(utils::read_bitmask<unsigned>(L, lua_gettop(L)));
        lua_pop(L, 1);
    }

    if(helper.try_push("drops")) {
        patch.drops = parse_drops(L, lua_gettop(L), ctx);
        lua_pop(L, 1);
    }

    return patch;
}

static void parse_definition(const FieldHelper& helper, ModContext* ctx, BlockDefinition& def) noexcept
{
    if(helper.try_push("render")) {
        def.render = static_cast<block_render>(luaL_checkinteger(helper.L, -1));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("model_name")) {
        def.model_name = Identifier::from_string(luaL_checkstring(helper.L, -1), ctx->name_space());
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("model_offset")) {
        def.model_offset = utils::read_vector3f(helper.L, lua_gettop(helper.L)) / 16.0f;
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("bcoll_name")) {
        def.bcoll_name = Identifier::from_string(luaL_checkstring(helper.L, -1), ctx->name_space());
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("bcoll_offset")) {
        def.bcoll_offset = utils::read_vector3f(helper.L, lua_gettop(helper.L)) / 16.0f;
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("animated")) {
        def.animated = static_cast<bool>(lua_toboolean(helper.L, -1));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("textures")) {
        auto textures_idx = lua_gettop(helper.L);

        def.textures.clear();
        def.textures.reserve(lua_rawlen(helper.L, textures_idx));

        lua_pushnil(helper.L);

        while(lua_next(helper.L, textures_idx)) {
            auto slot = std::string(luaL_checkstring(helper.L, -2));
            auto count = lua_rawlen(helper.L, -1);

            std::vector<Identifier> variants;
            variants.reserve(count);

            for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
                lua_rawgeti(helper.L, -1, i);
                variants.emplace_back(Identifier::from_string(luaL_checkstring(helper.L, -1), ctx->name_space()));
                lua_pop(helper.L, 1);
            }

            def.textures.insert_or_assign(std::move(slot), std::move(variants));

            lua_pop(helper.L, 1);
        }

        lua_pop(helper.L, 1);
    }

    if(helper.try_push("health")) {
        def.health = static_cast<unsigned>(luaL_checkinteger(helper.L, -1));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("sound")) {
        def.sound_set = Identifier::from_string(luaL_checkstring(helper.L, -1), ctx->name_space());
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("emission")) {
        def.emission = static_cast<block_light_type>(luaL_checkinteger(helper.L, -1));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("dissipation")) {
        def.dissipation = static_cast<block_light_type>(luaL_checkinteger(helper.L, -1));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("touch")) {
        def.touch = static_cast<block_touch>(luaL_checkinteger(helper.L, -1));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("touch_coeffs")) {
        def.touch_coeffs = utils::read_vector3f(helper.L, lua_gettop(helper.L));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("tags")) {
        def.tags = static_cast<block_tag_bit>(utils::read_bitmask<unsigned>(helper.L, lua_gettop(helper.L)));
        lua_pop(helper.L, 1);
    }

    if(helper.try_push("drops")) {
        def.drops = parse_drops(helper.L, lua_gettop(helper.L), ctx);
        lua_pop(helper.L, 1);
    }
}

static void parse_state_hint(lua_State* L, int entry_idx, BlockStateDecl& decl, BlockFamily& family) noexcept
{
    lua_getfield(L, entry_idx, "hint");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    auto hint_idx = lua_gettop(L);
    auto count = lua_rawlen(L, hint_idx);

    decl.hint.clear();
    decl.hint.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, hint_idx, i);
        decl.hint.push_back(family.state_hash(luaL_checkstring(L, -1)));
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

static void parse_states(lua_State* L, int idx, BlockFamily& family) noexcept
{
    lua_pushnil(L);

    while(lua_next(L, idx)) {
        auto state_name = std::string(luaL_checkstring(L, -2));
        auto entry_idx = lua_gettop(L);

        BlockStateDecl decl {};

        lua_getfield(L, entry_idx, "default");
        decl.default_value = family.state_hash(luaL_checkstring(L, -1));
        lua_pop(L, 1);

        parse_state_hint(L, entry_idx, decl, family);

        auto key = static_cast<blockstate_key_type>(utils::crc64(state_name.data(), state_name.size()));
        family.states.insert_or_assign(key, std::move(decl));

        lua_pop(L, 1);
    }
}

static void parse_variant_when(lua_State* L, int entry_idx, BlockVariantRule& rule, BlockFamily& family) noexcept
{
    lua_getfield(L, entry_idx, "when");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    auto when_idx = lua_gettop(L);

    lua_pushnil(L);

    while(lua_next(L, when_idx)) {
        auto state_name = std::string(luaL_checkstring(L, -2));
        auto value_str = luaL_checkstring(L, -1);

        auto key = static_cast<blockstate_key_type>(utils::crc64(state_name.data(), state_name.size()));
        auto value = family.state_hash(value_str);

        rule.when.insert_or_assign(key, value);

        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

static void parse_variant_overrides(lua_State* L, int entry_idx, BlockVariantRule& rule, ModContext* ctx) noexcept
{
    lua_getfield(L, entry_idx, "overrides");

    if(lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    rule.overrides = parse_overrides(L, lua_gettop(L), ctx);

    lua_pop(L, 1);
}

static void parse_variants(lua_State* L, int idx, ModContext* ctx, BlockFamily& family) noexcept
{
    auto count = lua_rawlen(L, idx);

    family.variants.clear();
    family.variants.reserve(count);

    for(lua_Integer i = 1; i <= static_cast<lua_Integer>(count); ++i) {
        lua_rawgeti(L, idx, i);

        auto entry_idx = lua_gettop(L);

        BlockVariantRule rule {};
        parse_variant_when(L, entry_idx, rule, family);
        parse_variant_overrides(L, entry_idx, rule, ctx);

        family.variants.push_back(std::move(rule));

        lua_pop(L, 1);
    }
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

    auto id = Identifier::from_string(raw_name, ctx->name_space());

    if(!id.is_valid()) {
        return luaL_error(L, "blocks.add: malformed name: %s", raw_name);
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

    FieldHelper helper(L, def_idx, proto_idx);
    parse_definition(helper, ctx, def);

    // a block that resolves into variants is allowed to omit model_name at
    // the top level - each variants[].overrides is expected to supply its
    // own (eg. a slab's base registration has no sensible single model)
    auto has_variants = helper.try_push("variants");

    if(has_variants) {
        lua_pop(L, 1);
    }

    if(!has_variants && def.render != BLOCK_RENDER_NONE && def.model_name.is_empty()) {
        return luaL_error(L, "blocks.add: [%s]: model_name is required unless render is blocks.RENDER_NONE", id.full_string().data());
    }

    auto block_id = ctx->register_block(id, def);
    auto has_checked_key = has_variants;

    for(const auto& key : std::array { "states", "variants", "on_rtick", "on_stick", "on_place", "on_break", "on_interact" }) {
        if(helper.try_push(key)) {
            has_checked_key = true;
            lua_pop(L, 1);
            break;
        }
    }

    if(has_checked_key) {
        BlockFamily family {};
        family.name = id;
        family.base_id = block_id;

        if(helper.try_push("states")) {
            parse_states(L, lua_gettop(L), family);
            lua_pop(L, 1);
        }

        if(helper.try_push("variants")) {
            parse_variants(L, lua_gettop(L), ctx, family);
            lua_pop(L, 1);
        }

        family.on_rtick = parse_callback(L, def_idx, "on_rtick", ctx);
        family.on_stick = parse_callback(L, def_idx, "on_stick", ctx);
        family.on_place = parse_callback(L, def_idx, "on_place", ctx);
        family.on_break = parse_callback(L, def_idx, "on_break", ctx);
        family.on_interact = parse_callback(L, def_idx, "on_interact", ctx);

        auto family_id = ctx->register_block_family(std::move(family));
        ctx->set_block_family(block_id, family_id);
    }

    lua_pushinteger(L, block_id);
    return 1;
}

void scripting::open_blocks_api(std::shared_ptr<lua_State>& lua, ModContext* mod_ctx) noexcept
{
    assert(lua);
    assert(mod_ctx);

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

    lua_pushlightuserdata(L, mod_ctx);
    lua_pushcclosure(L, &api_get, 1);
    lua_setfield(L, -2, "get");

    lua_pushcfunction(L, &api_has_tag);
    lua_setfield(L, -2, "has_tag");

    lua_pushlightuserdata(L, mod_ctx);
    lua_pushcclosure(L, &api_add, 1);
    lua_setfield(L, -2, "add");

    lua_setglobal(L, "blocks");
}
