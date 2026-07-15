#ifndef EAE3F86A_3F25_4073_80EF_D5CD0289B1EF
#define EAE3F86A_3F25_4073_80EF_D5CD0289B1EF

#include "core/identifier.hh"

#include "shared/block.hh"

enum biome_realm : unsigned {
    BIOME_REALM_SURFACE = 0,
    BIOME_REALM_CAVE,
    BIOME_REALM_DEEP,
    BIOME_REALM_SKY,
    BIOME_REALM__COUNT,
};

constexpr static int BIOME_TEMP_MIN = 200;
constexpr static int BIOME_TEMP_MAX = 330;
constexpr static int BIOME_HUMID_MIN = 0;
constexpr static int BIOME_HUMID_MAX = 100;
constexpr static int BIOME_AXIS_MIN = 0;
constexpr static int BIOME_AXIS_MAX = 100;

using biome_id_type = std::uint32_t;
constexpr static biome_id_type BIOME_ID_NULL = 0;
constexpr static biome_id_type BIOME_ID_MAX = std::numeric_limits<biome_id_type>::max();

struct BiomeBlockRef final {
    Identifier name;
    block_id_type id { BLOCK_ID_NULL };
};

struct BiomeStratum final {
    int depth;
    BiomeBlockRef block;
};

struct BiomeBlockPalette final {
    BiomeBlockRef base;
    BiomeBlockRef filler;
    BiomeBlockRef surface;
    BiomeBlockRef fluid;
    BiomeBlockRef ceiling;
};

struct BiomeFeatureEntry final {
    Identifier feature_id;
    float chance { 0.5f };
    bool requires_sky { false };
    bool requires_floor { false };
    std::optional<int> min_depth;
    std::optional<int> max_depth;
    std::optional<int> min_altitude;
    std::optional<int> max_altitude;
};

struct BiomeDefinition final {
    Identifier name;
    biome_realm realm { BIOME_REALM_SURFACE };

    int temperature { 0 };
    int humidity { 0 };
    int extra_axis { 0 };

    int priority { 0 };

    BiomeBlockPalette blocks {};
    std::vector<BiomeStratum> strata {};
    std::vector<BiomeFeatureEntry> features {};
};

#endif /* EAE3F86A_3F25_4073_80EF_D5CD0289B1EF */
