#ifndef B71DB4FE_395D_4792_BE70_67C2530260E4
#define B71DB4FE_395D_4792_BE70_67C2530260E4

#include "shared/world/fluid.hh"

class ModContext;

namespace fluid_registry
{
std::span<const FluidDefinition> all_definitions(void);
} // namespace fluid_registry

namespace fluid_registry
{
void resolve_tints(void);
} // namespace fluid_registry

namespace fluid_registry
{
void commit(ModContext& ctx);
void purge(void);
} // namespace fluid_registry

namespace fluid_registry
{
fluid_id_type find(const Identifier& id);
} // namespace fluid_registry

namespace fluid_registry
{
std::optional<Identifier> name_of(fluid_id_type id);
} // namespace fluid_registry

namespace fluid_registry
{
const FluidDefinition* find_definition(fluid_id_type id);
const FluidDefinition* find_definition(const Identifier& id);
} // namespace fluid_registry

#endif /* B71DB4FE_395D_4792_BE70_67C2530260E4 */
