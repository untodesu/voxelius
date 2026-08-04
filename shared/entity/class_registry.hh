#ifndef CF0A7378_F458_4745_A82A_0723299A9B7F
#define CF0A7378_F458_4745_A82A_0723299A9B7F

#include "shared/entity/class.hh"

class ModContext;

namespace class_registry
{
std::span<const ClassDefinition> all_definitions(void);
std::uint64_t checksum(void);
} // namespace class_registry

namespace class_registry
{
void commit(ModContext& ctx);
void purge(void);
} // namespace class_registry

namespace class_registry
{
class_id_type find(const Identifier& id);
} // namespace class_registry

namespace class_registry
{
std::optional<Identifier> name_of(class_id_type id);
} // namespace class_registry

namespace class_registry
{
const ClassDefinition* find_definition(class_id_type id);
const ClassDefinition* find_definition(const Identifier& id);
} // namespace class_registry

#endif /* CF0A7378_F458_4745_A82A_0723299A9B7F */
