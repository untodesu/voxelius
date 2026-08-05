#include "shared/pch.hh"

#include "shared/component/velocity.hh"

#include "core/buffer.hh"

#include "shared/component/stasis.hh"
#include "shared/component/transform.hh"
#include "shared/entity/component_registry.hh"
#include "shared/globals.hh"
#include "shared/utils/lua.hh"
#include "shared/world/world.hh"

static std::any velocity_parse(lua_State* L, int config_idx)
{
    return std::monostate {};
}

static void velocity_attach(entt::entity entity)
{
    Velocity_Component velocity {};
    velocity.value = Eigen::Vector3f::Zero();

    world::basic_entities.emplace_or_replace<Velocity_Component>(entity, std::move(velocity));
}

static bool velocity_update(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    auto& current = world::basic_entities.get<Velocity_Component>(entity);
    auto value = utils::opt_fvec<3>(L, kv_idx, current.value.cast<lua_Number>());

    if(!value.has_value()) {
        return false;
    }

    world::basic_entities.patch<Velocity_Component>(entity, [&](auto& velocity) {
        velocity.value = value.value().cast<float>();
    });

    return true;
}

static void velocity_encode(entt::entity entity, WriteBuffer& buffer)
{
    const auto& velocity = world::basic_entities.get<Velocity_Component>(entity);
    buffer.write_vector<float, 3>(velocity.value);
}

static void velocity_decode(entt::entity entity, ReadBuffer& buffer)
{
    world::basic_entities.patch<Velocity_Component>(entity, [&](auto& velocity) {
        velocity.value = buffer.read_vector<float, 3>();
    });
}

void Velocity_Component::register_component(void)
{
    ComponentDefinition def {};

    def.parse = &velocity_parse;
    def.attach = &velocity_attach;
    def.update = &velocity_update;

    def.net_encode = &velocity_encode;
    def.net_decode = &velocity_decode;
    def.save_encode = &velocity_encode;
    def.save_decode = &velocity_decode;

    component_registry::add("velocity", std::move(def));
}

void Velocity_Component::fixed_update(void)
{
    auto view = world::basic_entities.view<Velocity_Component, Transform>(entt::exclude<Stasis_Component>);

    for(auto [entity, velocity, transform] : view.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
