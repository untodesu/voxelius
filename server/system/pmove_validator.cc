#include "server/pch.hh"

#include "server/system/pmove_validator.hh"

#include "core/buffer.hh"
#include "core/config/map.hh"
#include "core/config/ref.hh"

#include "shared/component/move_mode.hh"
#include "shared/component/transform.hh"
#include "shared/component/velocity.hh"
#include "shared/entity/component_map.hh"
#include "shared/net/packet_entity.hh"
#include "shared/net/protocol.hh"
#include "shared/utils/coord.hh"

#include "server/globals.hh"
#include "server/net/sessions.hh"

static config::Ref<float> s_pos_threshold { 10.0f };
static config::Ref<float> s_vel_threshold { 10.0f };

static void send_full_update(entt::entity entity)
{
    static EntityPatch_Packet packet;

    auto transform_id = component_map::from_type<Transform>();
    auto velocity_id = component_map::from_type<Velocity>();
    assert(transform_id && velocity_id);

    WriteBuffer transform_buffer;
    component_map::encode_net(transform_id, entity, transform_buffer);
    EntityPatch_Packet::Component transform_component;
    transform_component.id = transform_id;
    transform_component.data = std::move(transform_buffer.take());

    WriteBuffer velocity_buffer;
    component_map::encode_net(velocity_id, entity, velocity_buffer);
    EntityPatch_Packet::Component velocity_component;
    velocity_component.id = velocity_id;
    velocity_component.data = std::move(velocity_buffer.take());

    packet.entity = entity;
    packet.components.clear();
    packet.components.emplace_back(std::move(transform_component));
    packet.components.emplace_back(std::move(velocity_component));

    auto& ref = globals::registry.get<SessionRef>(entity);
    protocol::send(packet, ref.ptr->peer);
}

void pmove_validator::init(void)
{
    s_pos_threshold.bind(globals::server_config, "pmove.pos_threshold");
    s_vel_threshold.bind(globals::server_config, "pmove.vel_threshold");
}

void pmove_validator::fixed_update_late(void)
{
    auto view = globals::registry.view<MoveMode, SessionRef, SimulatedMoveData, Transform, Velocity>();

    for(const auto [entity, move_mode, ref, sim_data, transform, velocity] : view.each()) {
        BlockPos sim_bpos = utils::to_block(sim_data.chunk, sim_data.local.cast<LocalPos::value_type>());
        BlockPos cur_bpos = utils::to_block(transform.chunk, transform.local.cast<LocalPos::value_type>());

        Eigen::Vector3f delta_vel = sim_data.velocity - velocity.value;
        BlockPos delta_bpos = sim_bpos - cur_bpos;

        auto dist_vel = delta_vel.norm();
        auto dist_bpos = delta_bpos.cast<float>().norm();

        auto rubberband = false;
        rubberband = rubberband || dist_vel > s_vel_threshold.value();
        rubberband = rubberband || dist_bpos > s_pos_threshold.value();

        if(rubberband) {
            LOG_WARNING("rubberbanding entity {}: dist_vel={} dist_bpos={}", static_cast<std::uint64_t>(entity), dist_vel, dist_bpos);

            send_full_update(entity);
            continue;
        }

        // The server agrees with what the client simulated; adopt the
        // client's viewpoint instead of trusting our own lagged simulation
        globals::registry.patch<Transform>(entity, [&](Transform& t) {
            t.chunk = sim_data.chunk;
            t.local = sim_data.local;
        });

        globals::registry.patch<Velocity>(entity, [&](Velocity& v) {
            v.value = sim_data.velocity;
        });
    }
}
