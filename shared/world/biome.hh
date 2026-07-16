#ifndef BCF648BF_4565_4F82_9C42_0491F751D447
#define BCF648BF_4565_4F82_9C42_0491F751D447

#include "core/identifier.hh"

#include "shared/world/block.hh"

enum biome_realm : unsigned {
    BIOME_REALM_SURFACE = 0,
    BIOME_REALM_UNDERGROUND,
    BIOME_REALM_THE_DEPTHS,
    BIOME_REALM_SKY,
    BIOME_REALM_COUNT,
};

using biome_id_type = std::uint32_t;
constexpr static biome_id_type BIOME_ID_NULL = 0;
constexpr static biome_id_type BIOME_ID_MAX = std::numeric_limits<biome_id_type>::max();

struct BiomeScatterEntry final {
    Identifier feature;
    float chance;
    block_tag_bit need_above;
    block_tag_bit need_below;
};

struct BiomePaletteEntry final {
    Identifier name;
    emhash8::HashMap<std::string, std::string> states;
    block_id_type cached;
};

struct BiomeDefinition {
    BiomeDefinition(void) = default;

    biome_realm realm;

    std::uint8_t lut_temp;
    std::uint8_t lut_humd;
    std::uint8_t lut_axis;

    unsigned priority;

    BiomePaletteEntry palette_empty;
    BiomePaletteEntry palette_basic;
    BiomePaletteEntry palette_filler;
    BiomePaletteEntry palette_surface;
    BiomePaletteEntry palette_fluid;

    std::vector<BiomeScatterEntry> scatter;
};

#endif /* BCF648BF_4565_4F82_9C42_0491F751D447 */
