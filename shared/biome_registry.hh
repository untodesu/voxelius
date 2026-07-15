#ifndef A3D8E1C4_6B2F_4E9A_8C7D_2F5B9E0A1C3D
#define A3D8E1C4_6B2F_4E9A_8C7D_2F5B9E0A1C3D

#include "shared/biome.hh"

class ModContext;

namespace biome_registry
{
std::span<const BiomeDefinition> all_definitions(void);
} // namespace biome_registry

namespace biome_registry
{
void commit(ModContext& ctx);
void purge(void);
} // namespace biome_registry

namespace biome_registry
{
biome_id_type find(const Identifier& id);
} // namespace biome_registry

namespace biome_registry
{
std::optional<Identifier> name_of(biome_id_type id);
} // namespace biome_registry

namespace biome_registry
{
const BiomeDefinition* find_definition(biome_id_type id);
const BiomeDefinition* find_definition(const Identifier& id);
} // namespace biome_registry

#endif /* A3D8E1C4_6B2F_4E9A_8C7D_2F5B9E0A1C3D */
