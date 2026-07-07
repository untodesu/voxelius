#ifndef C35E197B_F82F_423A_8226_7EB5C9E7FEEC
#define C35E197B_F82F_423A_8226_7EB5C9E7FEEC

#include "shared/block.hh"

class ModContext;

using blockstate_key_type = std::uint64_t;
using blockstate_val_type = std::uint64_t;

struct BlockStateDecl final {
    blockstate_val_type default_value;
    std::vector<blockstate_val_type> hint; // load-time only
};

struct BlockOverridePatch final {
    static BlockDefinition apply(BlockDefinition base, const BlockOverridePatch& patch) noexcept;

    std::optional<block_render> render;
    std::optional<std::unordered_map<std::string, std::vector<Identifier>>> textures;
    std::optional<bool> animated;

    std::optional<Identifier> model_name;
    std::optional<Eigen::Vector3f> model_offset;

    std::optional<Identifier> bcoll_name;
    std::optional<Eigen::Vector3f> bcoll_offset;

    std::optional<unsigned> health;

    std::optional<Identifier> sound_set;
    std::optional<block_light_type> emission;
    std::optional<block_light_type> dissipation;

    std::optional<block_touch> touch;
    std::optional<Eigen::Vector3f> touch_coeffs;

    std::optional<block_tag_bit> tags;

    std::optional<std::vector<BlockDrop>> drops;
};

struct BlockVariantRule final {
    std::unordered_map<blockstate_key_type, blockstate_val_type> when;
    BlockOverridePatch overrides;
};

struct BlockCallback final {
    std::shared_ptr<lua_State> state { nullptr };
    int callback_ref { LUA_NOREF };
};

struct BlockFamily final {
    Identifier name;
    block_id_type base_id;

    std::unordered_map<blockstate_key_type, BlockStateDecl> states;
    std::unordered_map<blockstate_val_type, std::string> state_values;
    std::vector<BlockVariantRule> variants;

    BlockCallback on_rtick;
    BlockCallback on_stick;
    BlockCallback on_place;
    BlockCallback on_break;
    BlockCallback on_interact;

    std::unordered_map<std::uint64_t, block_id_type> resolved_states; // cache of resolved states

    blockstate_val_type state_hash(std::string_view string) noexcept;
    std::string_view state_value(blockstate_val_type value) noexcept;
};

namespace block_registry
{
std::span<const BlockDefinition> all_definitions(void) noexcept;
std::span<const BlockFamily> all_families(void) noexcept;
} // namespace block_registry

namespace block_registry
{
void commit(ModContext& ctx) noexcept;
void purge(void) noexcept;
} // namespace block_registry

namespace block_registry
{
block_id_type find(const Identifier& id) noexcept;
} // namespace block_registry

namespace block_registry
{
const BlockDefinition* find_definition(block_id_type id) noexcept;
const BlockDefinition* find_definition(const Identifier& id) noexcept;
const BlockFamily* find_family(block_family_id_type id) noexcept;
const BlockFamily* find_family(const Identifier& id) noexcept;
} // namespace block_registry

namespace block_registry
{
bool has_tag_all(block_id_type id, block_tag_bit tag_bits) noexcept;
bool has_tag_any(block_id_type id, block_tag_bit tag_bits) noexcept;
} // namespace block_registry

namespace block_registry
{
block_id_type resolve_variant(block_id_type curr_id, const std::unordered_map<blockstate_key_type, blockstate_val_type>& map) noexcept;
} // namespace block_registry

#endif /* C35E197B_F82F_423A_8226_7EB5C9E7FEEC */
