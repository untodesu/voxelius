#include "shared/pch.hh"

#include "shared/component/head.hh"

#include "core/buffer.hh"

#include "shared/entity/component_registry.hh"
#include "shared/utils/lua.hh"
#include "shared/world/world.hh"

static std::any head_parse(lua_State* L, int config_idx)
{
    auto offset = utils::opt_fvec<3>(L, config_idx, { 0.0, 0.0, 0.0 });

    if(!offset.has_value()) {
        return std::any {};
    }

    return offset.value().cast<float>();
}

static void head_attach(entt::entity entity)
{
    Head_Component head {};
    head.offset = Eigen::Vector3f::Zero();
    head.angles = Eigen::Vector3f::Zero();

    world::basic_entities.emplace_or_replace<Head_Component>(entity, std::move(head));
}

static bool head_update(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    auto& current = world::basic_entities.get<Head_Component>(entity);
    auto angles = utils::opt_fvec<3>(L, kv_idx, current.angles.cast<lua_Number>());

    if(!angles.has_value()) {
        return false;
    }

    Eigen::Vector3f offset;

    if(std::any_cast<const Eigen::Vector3f>(&config)) {
        offset = std::any_cast<const Eigen::Vector3f>(config);
    }
    else {
        offset = current.offset;
    }

    world::basic_entities.patch<Head_Component>(entity, [&](auto& head) {
        head.angles = angles.value().cast<float>();
        head.offset = offset;
    });

    return true;
}

static void head_encode(entt::entity entity, WriteBuffer& buffer)
{
    const auto& head = world::basic_entities.get<Head_Component>(entity);
    buffer.write_vector<float, 3>(head.angles);
}

static void head_decode(entt::entity entity, ReadBuffer& buffer)
{
    world::basic_entities.patch<Head_Component>(entity, [&](auto& head) {
        head.angles = buffer.read_vector<float, 3>();
    });
}

void Head_Component::register_component(void)
{
    ComponentDefinition def {};

    def.parse = &head_parse;

    def.attach = &head_attach;
    def.update = &head_update;

    def.net_encode = &head_encode;
    def.net_decode = &head_decode;
    def.save_encode = &head_encode;
    def.save_decode = &head_decode;

    component_registry::add("head", def);
}
