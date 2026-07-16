#ifndef EE3ADE49_8032_436B_9D23_B5D3D25A8288
#define EE3ADE49_8032_436B_9D23_B5D3D25A8288

#include "shared/world/biome.hh"

namespace biome_lut
{
void generate(void);
} // namespace biome_lut

namespace biome_lut
{
const BiomeDefinition* find(biome_realm realm, std::uint8_t temp, std::uint8_t humd, std::uint8_t axis);
} // namespace biome_lut

#endif /* EE3ADE49_8032_436B_9D23_B5D3D25A8288 */
