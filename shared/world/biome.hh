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

struct ScatterEntry final {
    Identifier feature;
    float chance;
    block_tag_bit need_above;
    block_tag_bit need_below;
};

struct BiomeDefinition {
    BiomeDefinition(void) = default;

    biome_realm realm;

    std::uint8_t lut_temp;
    std::uint8_t lut_humd;
    std::uint8_t lut_axis;

    unsigned priority;

    std::string palette_empty;
    std::string palette_basic;
    std::string palette_filler;
    std::string palette_surface;
    std::string palette_fluid;

    block_id_type block_empty;
    block_id_type block_basic;
    block_id_type block_filler;
    block_id_type block_surface;
    block_id_type block_fluid;

    std::vector<ScatterEntry> scatter;
};

#endif /* BCF648BF_4565_4F82_9C42_0491F751D447 */
