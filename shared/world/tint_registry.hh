#ifndef E8FDAC6F_32F8_4149_B765_C7701B364C06
#define E8FDAC6F_32F8_4149_B765_C7701B364C06

#include "shared/world/tint.hh"

class Identifier;
class ModContext;

namespace tint_registry
{
std::span<const TintDefinition> all_definitions(void);
std::uint64_t checksum(void);
} // namespace tint_registry

namespace tint_registry
{
void commit(ModContext& ctx);
void purge(void);
} // namespace tint_registry

namespace tint_registry
{
tint_id_type find(const Identifier& id);
} // namespace tint_registry

namespace tint_registry
{
std::optional<Identifier> name_of(tint_id_type id);
} // namespace tint_registry

namespace tint_registry
{
const TintDefinition* find_definition(tint_id_type id);
const TintDefinition* find_definition(const Identifier& id);
} // namespace tint_registry

#endif /* E8FDAC6F_32F8_4149_B765_C7701B364C06 */
