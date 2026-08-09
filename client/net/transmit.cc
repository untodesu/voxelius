#include "client/pch.hh"

#include "client/net/transmit.hh"

#include "shared/component/transform.hh"
#include "shared/component/velocity.hh"
#include "shared/net/packet_player.hh"
#include "shared/net/protocol.hh"
#include "shared/system/pmove.hh"

#include "client/globals.hh"

void transmit::fixed_update_late(void)
{
    if(!globals::peer || !globals::registry.valid(globals::player)) {
        return;
    }

    const auto& transform = globals::registry.get<Transform>(globals::player);
    const auto& velocity = globals::registry.get<Velocity>(globals::player);
    const auto* move_data = globals::registry.try_get<MoveData>(globals::player);

    PlayerMoveData_Packet packet;
    packet.simulated_cpos = transform.chunk;
    packet.simulated_lpos = transform.local;
    packet.velocity = velocity.value;
    packet.wishdir = move_data ? move_data->wishdir : Eigen::Vector3f::Zero();

    protocol::send(packet, globals::peer);
}
