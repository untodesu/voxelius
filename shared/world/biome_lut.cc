#include "shared/pch.hh"

#include "shared/world/biome_lut.hh"

#include "shared/world/biome_registry.hh"

constexpr static std::size_t LUT_SIZE = 100;
constexpr static std::size_t LUT_TOTAL = LUT_SIZE * LUT_SIZE * LUT_SIZE;
constexpr static std::size_t LUT_COUNT = static_cast<std::size_t>(BIOME_REALM_COUNT);

using lut_table_type = std::array<biome_id_type, LUT_TOTAL>;

static std::array<lut_table_type, LUT_COUNT> s_tables;

static std::size_t table_index(std::uint8_t temp, std::uint8_t humd, std::uint8_t axis)
{
    auto temp_idx = static_cast<std::size_t>(temp);
    auto humd_idx = static_cast<std::size_t>(humd);
    auto axis_idx = static_cast<std::size_t>(axis);
    return temp_idx * LUT_SIZE * LUT_SIZE + humd_idx * LUT_SIZE + axis_idx;
}

static void table_fill(lut_table_type& table)
{
}

static void table_init(biome_realm realm, lut_table_type& table)
{
    table.fill(BIOME_ID_NULL);
}

void biome_lut::generate(void)
{
    table_init(BIOME_REALM_SURFACE, s_tables[BIOME_REALM_SURFACE]);
    table_init(BIOME_REALM_UNDERGROUND, s_tables[BIOME_REALM_UNDERGROUND]);
    table_init(BIOME_REALM_THE_DEPTHS, s_tables[BIOME_REALM_THE_DEPTHS]);
    table_init(BIOME_REALM_SKY, s_tables[BIOME_REALM_SKY]);

    table_fill(s_tables[BIOME_REALM_SURFACE]);
    table_fill(s_tables[BIOME_REALM_UNDERGROUND]);
    table_fill(s_tables[BIOME_REALM_THE_DEPTHS]);
    table_fill(s_tables[BIOME_REALM_SKY]);
}

const BiomeDefinition* biome_lut::find(biome_realm realm, std::uint8_t temp, std::uint8_t humd, std::uint8_t axis)
{
    if(realm >= BIOME_REALM_COUNT) {
        return nullptr;
    }

    auto& table = s_tables[static_cast<std::size_t>(realm)];
    auto index = table_index(temp, humd, axis);

    if(index > table.size()) {
        return nullptr;
    }

    return biome_registry::find_definition(table[index]);
}
