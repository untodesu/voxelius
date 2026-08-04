#include "shared/pch.hh"

#include "shared/component/velocity.hh"

#include "core/buffer.hh"

#include "shared/component/stasis.hh"
#include "shared/component/transform.hh"
#include "shared/entity/component_registry.hh"
#include "shared/globals.hh"
#include "shared/world/world.hh"

static std::any velocity_parse(lua_State* L, int config_idx)
{
    if(!lua_istable(L, config_idx)) {
        lua_pushfstring(L, "expected table, got %s", lua_typename(L, lua_type(L, config_idx)));
        return std::any {};
    }

    return std::monostate {};
}

static void velocity_spawn(entt::entity entity)
{
    Velocity_Component velocity {};
    velocity.value = Eigen::Vector3f::Zero();

    world::basic_entities.emplace_or_replace<Velocity_Component>(entity, std::move(velocity));
}

static bool velocity_apply(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    return true;
}

static void velocity_deserialize(entt::entity entity, ReadBuffer& buffer)
{
    auto& velocity = world::basic_entities.get<Velocity_Component>(entity);
    velocity.value = buffer.read_vector<float, 3>();

    world::basic_entities.patch<Velocity_Component>(entity);
}

static void velocity_serialize(entt::entity entity, WriteBuffer& buffer)
{
    const auto& velocity = world::basic_entities.get<Velocity_Component>(entity);
    buffer.write_vector<float, 3>(velocity.value);
}

void Velocity_Component::register_component(void)
{
    ComponentDefinition def {};
    def.parse = &velocity_parse;
    def.spawn = &velocity_spawn;
    def.apply = &velocity_apply;
    def.net_deserialize = &velocity_deserialize;
    def.sav_deserialize = &velocity_deserialize;
    def.net_serialize = &velocity_serialize;
    def.sav_serialize = &velocity_serialize;

    component_registry::add("velocity", std::move(def));
}

void Velocity_Component::fixed_update(void)
{
    auto view = world::basic_entities.view<Velocity_Component, Transform>(entt::exclude<Stasis_Component>);

    for(auto [entity, velocity, transform] : view.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
