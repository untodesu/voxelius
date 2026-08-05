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

static std::any transform_parse(lua_State* L, int config_idx)
{
    return std::monostate {};
}

static void transform_attach(entt::entity entity)
{
    Transform transform {};
    transform.chunk = ChunkPos::Zero();
    transform.local = Eigen::Vector3f::Zero();
    transform.angles = Eigen::Vector3f::Zero();

    world::basic_entities.emplace_or_replace<Transform>(entity, std::move(transform));
}

static bool transform_update(entt::entity entity, lua_State* L, int kv_idx, const std::any& config)
{
    auto& current = world::basic_entities.get<Transform>(entity);
    auto current_bpos = utils::to_block(current.chunk, current.local.cast<ChunkPos::value_type>());

    auto bpos = utils::opt_ivec<3>(L, kv_idx, "bpos", current_bpos.cast<lua_Integer>());

    if(!bpos.has_value()) {
        return false;
    }

    auto angs = utils::opt_fvec<3>(L, kv_idx, "angs", current.angles.cast<lua_Number>());

    if(!angs.has_value()) {
        return false;
    }

    world::basic_entities.patch<Transform>(entity, [&](auto& transform) {
        transform.chunk = utils::to_chunk(bpos.value().cast<BlockPos::value_type>());
        transform.local = utils::to_local(bpos.value().cast<BlockPos::value_type>()).cast<float>();
        transform.angles = angs.value().cast<float>();
    });

    return true;
}

static void transform_encode(entt::entity entity, WriteBuffer& buffer)
{
    const auto& transform = world::basic_entities.get<Transform>(entity);
    buffer.write_vector<std::int64_t, 3>(transform.chunk.cast<std::int64_t>());
    buffer.write_vector<float, 3>(transform.local);
    buffer.write_vector<float, 3>(transform.angles);
}

static void transform_decode(entt::entity entity, ReadBuffer& buffer)
{
    world::basic_entities.patch<Transform>(entity, [&](auto& transform) {
        transform.chunk = buffer.read_vector<std::int64_t, 3>().cast<ChunkPos::value_type>();
        transform.local = buffer.read_vector<float, 3>();
        transform.angles = buffer.read_vector<float, 3>();
    });
}

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

void Transform::register_component(void)
{
    ComponentDefinition def {};

    def.parse = &transform_parse;
    def.attach = &transform_attach;
    def.update = &transform_update;

    def.net_encode = &transform_encode;
    def.net_decode = &transform_decode;
    def.save_encode = &transform_encode;
    def.save_decode = &transform_decode;

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
