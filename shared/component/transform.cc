#include "shared/pch.hh"

#include "shared/component/transform.hh"

#include "core/buffer.hh"

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/entity/class.hh"
#include "shared/entity/component_registry.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/lua.hh"
#include "shared/world/world.hh"

constexpr inline static void update_component(unsigned dim, Transform& component)
{
    if(component.local[dim] >= constant::CHUNK_SIZE) {
        component.local[dim] -= constant::CHUNK_SIZE;
        component.chunk[dim] += 1;
        return;
    }

    if(component.local[dim] < 0.0f) {
        component.local[dim] += constant::CHUNK_SIZE;
        component.chunk[dim] -= 1;
        return;
    }
}

static std::any transform_parse(lua_State* L, int config_idx)
{
    return std::monostate {};
}

static void transform_spawn(entt::entity entity)
{
    Transform transform {};
    transform.chunk = ChunkPos::Zero();
    transform.local = Eigen::Vector3f::Zero();
    transform.angles = Eigen::Vector3f::Zero();

    world::basic_entities.emplace_or_replace<Transform>(entity, std::move(transform));
}

static bool transform_apply(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    auto bpos = utils::opt_ivec<3>(L, kv_idx, "bpos", { 0, 0, 0 });

    if(!bpos.has_value()) {
        return false;
    }

    auto angles = utils::opt_fvec<3>(L, kv_idx, "angs", { 0.0, 0.0, 0.0 });

    if(!angles.has_value()) {
        return false;
    }

    auto& transform = world::basic_entities.get<Transform>(entity);
    transform.chunk = utils::to_chunk(bpos.value().cast<BlockPos::value_type>());
    transform.local = utils::to_local(bpos.value().cast<BlockPos::value_type>()).cast<float>();
    transform.angles = angles.value().cast<float>();

    return true;
}

static void transform_deserialize(entt::entity entity, ReadBuffer& buffer)
{
    auto& transform = world::basic_entities.get<Transform>(entity);
    transform.chunk = buffer.read_vector<std::int64_t, 3>().cast<ChunkPos::value_type>();
    transform.local = buffer.read_vector<float, 3>();
    transform.angles = buffer.read_vector<float, 3>();

    world::basic_entities.patch<Transform>(entity);
}

static void transform_serialize(entt::entity entity, WriteBuffer& buffer)
{
    const auto& transform = world::basic_entities.get<Transform>(entity);
    buffer.write_vector<std::int64_t, 3>(transform.chunk.cast<std::int64_t>());
    buffer.write_vector<float, 3>(transform.local);
    buffer.write_vector<float, 3>(transform.angles);
}

void Transform::register_component(void)
{
    ComponentDefinition def {};
    def.parse = &transform_parse;
    def.spawn = &transform_spawn;
    def.apply = &transform_apply;
    def.net_deserialize = &transform_deserialize;
    def.sav_deserialize = &transform_deserialize;
    def.net_serialize = &transform_serialize;
    def.sav_serialize = &transform_serialize;

    component_registry::add("transform", std::move(def));
}

void Transform::fixed_update(void)
{
    auto view = world::basic_entities.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        update_component(0U, transform);
        update_component(1U, transform);
        update_component(2U, transform);

        if(auto class_component = world::basic_entities.try_get<EntityClass_Component>(entity)) {
            LOG_INFO("entity {} ({}) transform update", static_cast<std::uint64_t>(entity), class_component->id.full_string());
        }
    }
}
