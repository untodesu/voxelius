#include "shared/pch.hh"

#include "shared/component/transform.hh"

#include "core/buffer.hh"

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/entity/class.hh"
#include "shared/entity/component_map.hh"
#include "shared/globals.hh"
#include "shared/utils/coord.hh"
#include "shared/utils/lua.hh"

constexpr inline static bool update_component(unsigned dim, Transform& component)
{
    if(component.local[dim] >= constant::CHUNK_SIZE) {
        component.local[dim] -= constant::CHUNK_SIZE;
        component.chunk[dim] += 1;
        return true;
    }

    if(component.local[dim] < 0.0f) {
        component.local[dim] += constant::CHUNK_SIZE;
        component.chunk[dim] -= 1;
        return true;
    }

    return false;
}

std::any Component<Transform>::prepare(lua_State* L, int config_idx)
{
    return std::monostate {};
}

void Component<Transform>::attach(entt::entity entity, const std::any& config)
{
    Transform transform {};
    transform.chunk = ChunkPos::Zero();
    transform.local = Eigen::Vector3f::Zero();
    transform.angles = Eigen::Vector3f::Zero();

    globals::registry.emplace_or_replace<Transform>(entity, std::move(transform));
}

bool Component<Transform>::patch(entt::entity entity, lua_State* L, int kv_idx)
{
    auto& current = globals::registry.get<Transform>(entity);
    auto current_bpos = utils::to_block(current.chunk, current.local.cast<ChunkPos::value_type>());

    auto bpos = utils::opt_ivec<3>(L, kv_idx, "bpos", current_bpos.cast<lua_Integer>());

    if(!bpos.has_value()) {
        return false;
    }

    auto angs = utils::opt_fvec<3>(L, kv_idx, "angs", current.angles.cast<lua_Number>());

    if(!angs.has_value()) {
        return false;
    }

    globals::registry.patch<Transform>(entity, [&](Transform& transform) {
        transform.chunk = utils::to_chunk(bpos.value().cast<BlockPos::value_type>());
        transform.local = utils::to_local(bpos.value().cast<BlockPos::value_type>()).cast<float>();
        transform.angles = angs.value().cast<float>();
    });

    return true;
}

void Component<Transform>::encode_net(entt::entity entity, WriteBuffer& buffer)
{
    const auto& transform = globals::registry.get<Transform>(entity);
    buffer.write_vector<std::int64_t, 3>(transform.chunk.cast<std::int64_t>());
    buffer.write_vector<float, 3>(transform.local);
    buffer.write_vector<float, 3>(transform.angles);
}

void Component<Transform>::decode_net(entt::entity entity, ReadBuffer& buffer)
{
    globals::registry.patch<Transform>(entity, [&](Transform& transform) {
        transform.chunk = buffer.read_vector<std::int64_t, 3>().cast<ChunkPos::value_type>();
        transform.local = buffer.read_vector<float, 3>();
        transform.angles = buffer.read_vector<float, 3>();
    });
}

void Component<Transform>::encode_dat(entt::entity entity, WriteBuffer& buffer)
{
    encode_net(entity, buffer);
}

void Component<Transform>::decode_dat(entt::entity entity, ReadBuffer& buffer)
{
    decode_net(entity, buffer);
}

void Transform::register_component(void)
{
    component_map::add<Transform>("transform");

    globals::registry.on_update<Transform>().connect<&component_map::on_update<Transform>>();
}

void Transform::fixed_update(void)
{
    auto view = globals::registry.view<Transform>();

    for(auto [entity, transform] : view.each()) {
        auto changed = false;
        changed = update_component(0, transform) || changed;
        changed = update_component(1, transform) || changed;
        changed = update_component(2, transform) || changed;

        if(changed) {
            globals::registry.patch<Transform>(entity);
        }
    }
}
