#ifndef EE38FCD4_F793_4446_9DCF_44B1285F9330
#define EE38FCD4_F793_4446_9DCF_44B1285F9330

#include "shared/block_registry.hh"

namespace config
{
class Map;
} // namespace config

constexpr static std::string_view BUILTIN_MOD_NAME = "builtin";

enum class mod_status {
    PENDING, ///< The mod is pending to be loaded
    LOADING, ///< The mod is currently being loaded
    LOADED,  ///< The mod has been successfully loaded
    FAILED,  ///< The mod has failed to load
    SKIPPED, ///< One of the mod's dependencies has failed to load and the mod will be skipped
};

struct ModInfo final {
    static bool parse(const config::Map& map, ModInfo& modinfo);

    std::string name;
    std::vector<std::string> depends;

    std::string meta_author;   ///< Author or authors of the mod
    std::string meta_homepage; ///< Homepage or website of the mod
    std::string meta_tracker;  ///< Tracker or issue tracker of the mod
    std::string meta_license;  ///< Short SPDX-identifier of the mod's license

    std::string display_name; ///< Printable name of the mod
    std::string display_desc; ///< Printable description of the mod
};

class ModContext final {
public:
    explicit ModContext(ModInfo modinfo) noexcept;

    ModContext(const ModContext& other) = delete;
    ModContext& operator=(const ModContext& other) = delete;

    ModContext(ModContext&& other) noexcept = default;
    ModContext& operator=(ModContext&& other) noexcept = default;

    constexpr const ModInfo& modinfo(void) const noexcept;
    constexpr std::string_view name_space(void) const noexcept;
    constexpr mod_status status(void) const noexcept;

    bool initialize(void) noexcept;

    // non-owning: callers that need to keep a mod's Lua state alive past
    // ModContext's own lifetime (eg. BlockCallback) should copy this
    const std::shared_ptr<lua_State>& lua_state(void) const noexcept;

    block_id_type find_block(const Identifier& name) const noexcept;
    block_id_type register_block(const Identifier& name, BlockDefinition def) noexcept;
    block_family_id_type register_block_family(BlockFamily family) noexcept;

    // patches a block registered earlier in this same loading pass to
    // point at the family that was, in turn, only registerable after
    // the block itself (family.base_id needs the block's local id first)
    bool set_block_family(block_id_type id, block_family_id_type family) noexcept;

    std::vector<BlockDefinition> take_blocks(void) noexcept;
    std::vector<BlockFamily> take_block_families(void) noexcept;
    std::unordered_map<Identifier, block_id_type> take_block_names(void) noexcept;

private:
    ModInfo m_modinfo;
    mod_status m_status;
    std::shared_ptr<lua_State> m_lua_state;

    std::vector<BlockDefinition> m_blocks;
    std::vector<BlockFamily> m_block_families;
    std::unordered_map<Identifier, block_id_type> m_block_names;
};

constexpr const ModInfo& ModContext::modinfo(void) const noexcept
{
    return m_modinfo;
}

constexpr std::string_view ModContext::name_space(void) const noexcept
{
    return m_modinfo.name;
}

constexpr mod_status ModContext::status(void) const noexcept
{
    return m_status;
}

inline const std::shared_ptr<lua_State>& ModContext::lua_state(void) const noexcept
{
    return m_lua_state;
}

#endif /* EE38FCD4_F793_4446_9DCF_44B1285F9330 */
