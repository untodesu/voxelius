#ifndef EE38FCD4_F793_4446_9DCF_44B1285F9330
#define EE38FCD4_F793_4446_9DCF_44B1285F9330

#include "shared/entity/class_registry.hh"
#include "shared/world/biome_registry.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/fluid_registry.hh"
#include "shared/world/tint_registry.hh"

namespace config
{
class Map;
} // namespace config

enum class mod_status {
    PENDING, ///< The mod is pending to be loaded
    LOADING, ///< The mod is currently being loaded
    LOADED,  ///< The mod has been successfully loaded
    FAILED,  ///< The mod has failed to load
    SKIPPED, ///< One of the mod's dependencies has failed to load and the mod will be skipped
};

struct ModVersion final {
    static ModVersion parse(std::string_view string);
    static std::string to_string(const ModVersion& version);

    unsigned major { 0 };
    unsigned minor { 0 };
    unsigned patch { 0 };

    constexpr auto operator<=>(const ModVersion& other) const = default;
};

struct ModInfo final {
    static bool parse(const config::Map& map, ModInfo& modinfo);

    std::string name;
    ModVersion version;

    std::vector<std::pair<std::string, ModVersion>> hard_depends;
    std::vector<std::pair<std::string, ModVersion>> soft_depends;
    std::vector<std::pair<std::string, ModVersion>> conflicts;

    std::string meta_author;   ///< Author or authors of the mod
    std::string meta_homepage; ///< Homepage or website of the mod
    std::string meta_tracker;  ///< Tracker or issue tracker of the mod
    std::string meta_license;  ///< Short SPDX-identifier of the mod's license

    std::string display_name; ///< Printable name of the mod
    std::string display_desc; ///< Printable description of the mod
};

class ModContext final {
public:
    explicit ModContext(ModInfo modinfo);

    ModContext(const ModContext& other) = delete;
    ModContext& operator=(const ModContext& other) = delete;

    ModContext(ModContext&& other) = default;
    ModContext& operator=(ModContext&& other) = default;

    constexpr const ModInfo& modinfo(void) const;
    constexpr std::string_view name_space(void) const;
    constexpr mod_status status(void) const;

    void set_status(mod_status status);

    bool initialize(void);

    // non-owning: callers that need to keep a mod's Lua state alive past
    // ModContext's own lifetime (eg. BlockCallback) should copy this
    const std::shared_ptr<lua_State>& lua_state(void) const;

    block_id_type find_block(const Identifier& name) const;
    block_id_type register_block(const Identifier& name, BlockDefinition def);
    block_family_id_type register_block_family(BlockFamily family);

    // patches a block registered earlier in this same loading pass to
    // point at the family that was, in turn, only registerable after
    // the block itself (family.stem_id needs the block's local id first)
    bool set_block_family(block_id_type id, block_family_id_type family);

    std::vector<BlockDefinition> take_blocks(void);
    std::vector<BlockFamily> take_block_families(void);
    emhash8::HashMap<Identifier, block_id_type> take_block_names(void);

    biome_id_type find_biome(const Identifier& name) const;
    biome_id_type register_biome(const Identifier& name, BiomeDefinition def);

    std::vector<BiomeDefinition> take_biomes(void);
    emhash8::HashMap<Identifier, biome_id_type> take_biome_names(void);

    fluid_id_type find_fluid(const Identifier& name) const;
    fluid_id_type register_fluid(const Identifier& name, FluidDefinition def);

    std::vector<FluidDefinition> take_fluids(void);
    emhash8::HashMap<Identifier, fluid_id_type> take_fluid_names(void);

    tint_id_type find_tint(const Identifier& name) const;
    tint_id_type register_tint(const Identifier& name, TintDefinition def);

    std::vector<TintDefinition> take_tints(void);
    emhash8::HashMap<Identifier, tint_id_type> take_tint_names(void);

    class_id_type find_class(const Identifier& name) const;
    class_id_type register_class(const Identifier& name, ClassDefinition def);

    std::vector<ClassDefinition> take_classes(void);
    emhash8::HashMap<Identifier, class_id_type> take_class_names(void);

private:
    ModInfo m_modinfo;
    mod_status m_status;
    std::shared_ptr<lua_State> m_lua_state;

    std::vector<BlockDefinition> m_blocks;
    std::vector<BlockFamily> m_block_families;
    emhash8::HashMap<Identifier, block_id_type> m_block_names;

    std::vector<BiomeDefinition> m_biomes;
    emhash8::HashMap<Identifier, biome_id_type> m_biome_names;

    std::vector<FluidDefinition> m_fluids;
    emhash8::HashMap<Identifier, fluid_id_type> m_fluid_names;

    std::vector<TintDefinition> m_tints;
    emhash8::HashMap<Identifier, tint_id_type> m_tint_names;

    std::vector<ClassDefinition> m_classes;
    emhash8::HashMap<Identifier, class_id_type> m_class_names;
};

constexpr const ModInfo& ModContext::modinfo(void) const
{
    return m_modinfo;
}

constexpr std::string_view ModContext::name_space(void) const
{
    return m_modinfo.name;
}

constexpr mod_status ModContext::status(void) const
{
    return m_status;
}

inline const std::shared_ptr<lua_State>& ModContext::lua_state(void) const
{
    return m_lua_state;
}

#endif /* EE38FCD4_F793_4446_9DCF_44B1285F9330 */
