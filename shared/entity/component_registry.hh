#ifndef AFD5DBFC_92AA_4C82_85A9_F6FC3D3C4ADE
#define AFD5DBFC_92AA_4C82_85A9_F6FC3D3C4ADE

#include "shared/entity/component.hh"

namespace component_registry
{
component_id_type add(std::string_view name, ComponentDefinition def);
component_id_type find(std::string_view name);
} // namespace component_registry

namespace component_registry
{
std::string_view name_of(component_id_type id);
} // namespace component_registry

namespace component_registry
{
const ComponentDefinition* find_definition(std::string_view name);
const ComponentDefinition* find_definition(component_id_type id);
} // namespace component_registry

namespace component_registry
{
std::any parse(component_id_type id, lua_State* L, int config_idx);
} // namespace component_registry

namespace component_registry
{
bool attach(component_id_type id, entt::entity entity);
bool update(component_id_type id, entt::entity entity, lua_State* L, int kv_idx, const std::any& config);
} // namespace component_registry

namespace component_registry
{
bool net_encode(component_id_type id, entt::entity entity, WriteBuffer& buffer);
bool net_decode(component_id_type id, entt::entity entity, ReadBuffer& buffer);
} // namespace component_registry

namespace component_registry
{
bool save_encode(component_id_type id, entt::entity entity, WriteBuffer& buffer);
bool save_decode(component_id_type id, entt::entity entity, ReadBuffer& buffer);
} // namespace component_registry

#endif /* AFD5DBFC_92AA_4C82_85A9_F6FC3D3C4ADE */
