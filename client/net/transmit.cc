#include "client/pch.hh"

#include "client/net/transmit.hh"

#include "shared/component/head.hh"
#include "shared/component/transform.hh"
#include "shared/component/velocity.hh"
#include "shared/net/packet_player.hh"
#include "shared/net/protocol.hh"
#include "shared/system/pmove.hh"

#include "client/globals.hh"

void transmit::fixed_update_late(void)
{
    if(globals::peer && globals::registry.valid(globals::player)) {
        const auto head = globals::registry.try_get<Head>(globals::player);
        const auto transform = globals::registry.try_get<Transform>(globals::player);
        const auto velocity = globals::registry.try_get<Velocity>(globals::player);
        const auto move_data = globals::registry.try_get<MoveData>(globals::player);

        packet::Player_MoveData packet {};
        packet.simulated_cpos = ChunkPos::Zero();
        packet.camera_angles = Eigen::Vector3f::Zero();
        packet.simulated_lpos = Eigen::Vector3f::Zero();
        packet.velocity = Eigen::Vector3f::Zero();
        packet.wishdir = Eigen::Vector3f::Zero();

        if(head) {
            packet.camera_angles.x() = head->angles.x();
            packet.camera_angles.z() = head->angles.z();
        }

        if(transform) {
            packet.simulated_cpos = transform->chunk;
            packet.simulated_lpos = transform->local;
            packet.camera_angles.y() = transform->angles.y();
        }

        if(velocity) {
            packet.velocity = velocity->value;
        }

        if(move_data) {
            packet.wishdir = move_data->wishdir;
        }

        protocol::send(packet, globals::peer);
    }
}
