#include "shared/pch.hh"

#include "shared/component/velocity.hh"

#include "core/buffer.hh"

#include "shared/component/stasis.hh"
#include "shared/component/transform.hh"
#include "shared/entity/component_map.hh"
#include "shared/globals.hh"
#include "shared/utils/lua.hh"

std::any Component<Velocity>::prepare(lua_State* L, int config_idx)
{
    return std::monostate {};
}

void Component<Velocity>::attach(entt::entity entity)
{
    Velocity velocity {};
    velocity.value = Eigen::Vector3f::Zero();

    globals::registry.emplace_or_replace<Velocity>(entity, std::move(velocity));
}

bool Component<Velocity>::update(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    auto& current = globals::registry.get<Velocity>(entity);
    auto value = utils::opt_fvec<3>(L, kv_idx, current.value.cast<lua_Number>());

    if(!value.has_value()) {
        return false;
    }

    globals::registry.patch<Velocity>(entity, [&](auto& velocity) {
        velocity.value = value.value().cast<float>();
    });

    return true;
}

void Component<Velocity>::encode_net(entt::entity entity, WriteBuffer& buffer)
{
    const auto& velocity = globals::registry.get<Velocity>(entity);
    buffer.write_vector<float, 3>(velocity.value);
}

void Component<Velocity>::decode_net(entt::entity entity, ReadBuffer& buffer)
{
    globals::registry.patch<Velocity>(entity, [&](auto& velocity) {
        velocity.value = buffer.read_vector<float, 3>();
    });
}

void Component<Velocity>::encode_dat(entt::entity entity, WriteBuffer& buffer)
{
    encode_net(entity, buffer);
}

void Component<Velocity>::decode_dat(entt::entity entity, ReadBuffer& buffer)
{
    decode_net(entity, buffer);
}

void Velocity::register_component(void)
{
    component_map::add<Velocity>("velocity");

    globals::registry.on_construct<Velocity>().connect<&component_map::on_update<Velocity>>();
}

void Velocity::fixed_update(void)
{
    auto view = globals::registry.view<Velocity, Transform>(entt::exclude<Stasis>);

    for(auto [entity, velocity, transform] : view.each()) {
        transform.local += velocity.value * globals::fixed_frametime;
    }
}
