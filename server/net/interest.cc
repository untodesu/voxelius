#include "server/pch.hh"

#include "server/net/interest.hh"

#include "core/config/ref.hh"

#include "shared/component/transform.hh"
#include "shared/globals.hh"
#include "shared/net/packet_entity.hh"
#include "shared/net/packet_world.hh"
#include "shared/utils/view.hh"

#include "server/globals.hh"
#include "server/net/sessions.hh"

struct SessionView final {
    ENetPeer* peer;
    ChunkAlignedBox box;
};

config::Ref<unsigned> interest::view_distance { 8 };

static std::vector<SessionView> s_views;

void interest::init(void)
{
    view_distance.bind(globals::server_config, "interest.view_distance");
}

void interest::fixed_update_late(void)
{
    ZoneScoped;

    s_views.clear();

    for(const auto& session : sessions::all()) {
        if(globals::registry.valid(session.player)) {
            auto& transform = globals::registry.get<Transform>(session.player);
            auto radius = static_cast<ChunkPos::value_type>(view_distance.value());

            SessionView view {};
            view.peer = session.peer;
            view.box = utils::view_box(transform.chunk, radius);
            s_views.push_back(view);
        }
    }
}

template<typename T>
void interest::broadcast(const T& packet, const ChunkPos& origin, ENetPeer* except)
{
    for(const auto& view : s_views) {
        if(view.peer == except) {
            continue;
        }

        if(view.box.contains(origin)) {
            protocol::send(packet, view.peer);
        }
    }
}

template void interest::broadcast<packet::World_SetBlock>(const packet::World_SetBlock& packet, const ChunkPos& origin, ENetPeer* except);
template void interest::broadcast<packet::Entity_Spawn>(const packet::Entity_Spawn& packet, const ChunkPos& origin, ENetPeer* except);
template void interest::broadcast<packet::Entity_Patch>(const packet::Entity_Patch& packet, const ChunkPos& origin, ENetPeer* except);
template void interest::broadcast<packet::Entity_Remove>(const packet::Entity_Remove& packet, const ChunkPos& origin, ENetPeer* except);
