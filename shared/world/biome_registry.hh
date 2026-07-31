#ifndef B3926871_E908_46CF_99EB_D2E058C9DADE
#define B3926871_E908_46CF_99EB_D2E058C9DADE

#include "shared/world/biome.hh"

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
void resolve_palettes(void);
void resolve_features(void);
void resolve_tint_colors(void);
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

#endif /* B3926871_E908_46CF_99EB_D2E058C9DADE */
