#include "shared/pch.hh"

#include "shared/component/player.hh"

#include "shared/entity/component_map.hh"
#include "shared/globals.hh"

std::any Component<Player>::prepare(lua_State* L, int config_idx)
{
    return std::monostate {};
}

void Component<Player>::attach(entt::entity entity, const std::any& config)
{
    globals::registry.emplace<Player>(entity);
}

bool Component<Player>::patch(entt::entity entity, lua_State* L, int kv_idx)
{
    return true;
}

void Component<Player>::encode_net(entt::entity entity, WriteBuffer& buffer)
{
    // empty
}

void Component<Player>::decode_net(entt::entity entity, ReadBuffer& buffer)
{
    // empty
}

void Component<Player>::encode_dat(entt::entity entity, WriteBuffer& buffer)
{
    // empty
}

void Component<Player>::decode_dat(entt::entity entity, ReadBuffer& buffer)
{
    // empty
}

void Player::register_component(void)
{
    component_map::add<Player>("player");

    globals::registry.on_update<Player>().connect<&component_map::on_update<Player>>();
}
