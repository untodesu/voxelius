#ifndef F621D887_03BF_4930_817F_D7E18D8E2A55
#define F621D887_03BF_4930_817F_D7E18D8E2A55

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

#endif /* F621D887_03BF_4930_817F_D7E18D8E2A55 */
