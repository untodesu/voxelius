#include "shared/pch.hh"

#include "shared/component/head.hh"

#include "core/buffer.hh"

#include "shared/entity/component_registry.hh"
#include "shared/utils/lua.hh"
#include "shared/world/world.hh"

struct Head_Config final {
    Eigen::Vector3f offset;
};

static std::any head_parse(lua_State* L, int config_idx)
{
    if(!lua_istable(L, config_idx)) {
        lua_pushfstring(L, "expected table, got %s", lua_typename(L, lua_type(L, config_idx)));
        return std::any {};
    }

    auto offset = utils::opt_fvec<3>(L, config_idx, "offset", { 0.0, 0.0, 0.0 });

    if(!offset.has_value()) {
        return std::any {};
    }

    Head_Config config {};
    config.offset = offset.value().cast<float>();

    return config;
}

static void head_spawn(entt::entity entity)
{
    Head_Component head {};
    head.offset = Eigen::Vector3f::Zero();
    head.angles = Eigen::Vector3f::Zero();

    world::basic_entities.emplace_or_replace<Head_Component>(entity, std::move(head));
}

static bool head_apply(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    auto& my_config = std::any_cast<const Head_Config&>(config);

    auto angles = utils::opt_fvec<3>(L, kv_idx, "angles", { 0.0, 0.0, 0.0 });

    if(!angles.has_value()) {
        return false;
    }

    auto& head = world::basic_entities.get<Head_Component>(entity);
    head.offset = my_config.offset;
    head.angles = angles.value().cast<float>();

    return true;
}

static void head_deserialize(entt::entity entity, ReadBuffer& buffer)
{
    auto& head = world::basic_entities.get<Head_Component>(entity);
    head.angles = buffer.read_vector<float, 3>();

    world::basic_entities.patch<Head_Component>(entity);
}

static void head_serialize(entt::entity entity, WriteBuffer& buffer)
{
    const auto& head = world::basic_entities.get<Head_Component>(entity);
    buffer.write_vector<float, 3>(head.angles);
}

void Head_Component::register_component(void)
{
    ComponentDefinition def {};
    def.parse = &head_parse;
    def.spawn = &head_spawn;
    def.apply = &head_apply;
    def.net_deserialize = &head_deserialize;
    def.sav_deserialize = &head_deserialize;
    def.net_serialize = &head_serialize;
    def.sav_serialize = &head_serialize;

    component_registry::add("head", def);
}
