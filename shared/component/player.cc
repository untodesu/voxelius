#include "shared/pch.hh"

#include "shared/component/player.hh"

#include "shared/entity/component_registry.hh"
#include "shared/world/world.hh"

static std::any player_parse(lua_State* L, int config_idx)
{
    return std::monostate {};
}

static void player_attach(entt::entity entity)
{
    world::basic_entities.emplace<Player_Component>(entity);
}

static bool player_update(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    return true;
}

void Player_Component::register_component(void)
{
    ComponentDefinition def {};
    def.parse = player_parse;
    def.attach = player_attach;
    def.update = player_update;

    component_registry::add("player", def);
}
