#include "client/pch.hh"

#include "client/world/chunk_visibility.hh"

#include "shared/coord.hh"
#include "shared/net/packet_world.hh"
#include "shared/net/protocol.hh"
#include "shared/world/chunk.hh"
#include "shared/world/world.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/gui/slider.hh"
#include "client/net/session.hh"

struct ChunkRequest final {
    ChunkPos position;
    unsigned distance;
};

constexpr static std::size_t REQUESTS_PER_TICK = 48;

static ChunkAlignedBox s_bounds_curr;
static ChunkAlignedBox s_bounds_prev;
static std::vector<ChunkRequest> s_requests;

static void build_requests(void)
{
    s_requests.clear();

    const auto& min = s_bounds_curr.min();
    const auto& max = s_bounds_curr.max();

    for(auto cx = min.x(); cx <= max.x(); cx += 1) {
        for(auto cy = min.y(); cy <= max.y(); cy += 1) {
            for(auto cz = min.z(); cz <= max.z(); cz += 1) {
                auto cpos = ChunkPos(cx, cy, cz);

                auto want_request = false;
                want_request = want_request || nullptr == world::find_chunk(cpos);
                want_request = want_request || !s_bounds_prev.contains(cpos);

                if(want_request) {
                    ChunkRequest request {};
                    request.position = cpos;
                    request.distance = static_cast<unsigned>((cpos - camera::chunk).squaredNorm());
                    s_requests.emplace_back(std::move(request));
                }
            }
        }
    }

    std::sort(s_requests.begin(), s_requests.end(), [](const auto& a, const auto& b) {
        return a.distance > b.distance;
    });
}

void chunk_visibility::fixed_update_late(void)
{
    ZoneScoped;

    static std::vector<ChunkPos> to_remove;

    if(session::state == SESSION_INGAME) {
        auto radius = static_cast<ChunkPos::value_type>(camera::view_distance.value());
        s_bounds_curr.min() = camera::chunk - ChunkPos::Constant(radius);
        s_bounds_curr.max() = camera::chunk + ChunkPos::Constant(radius);

        if(!s_bounds_curr.isApprox(s_bounds_prev)) {
            build_requests();
        }

        for(std::size_t i = 0; s_requests.size() && i < REQUESTS_PER_TICK; ++i) {
            const auto& request = s_requests.back();

            packet::World_Request packet;
            packet.cpos = request.position;
            protocol::send(packet, globals::peer);

            s_requests.pop_back();
        }

        auto view = world::chunk_registry.view<Chunk_Component>();

        to_remove.clear();

        for(const auto [entity, component] : view.each()) {
            if(s_bounds_curr.contains(component.position)) {
                continue;
            }

            to_remove.push_back(component.position);
        }

        for(const auto& cpos : to_remove) {
            world::remove_chunk(cpos);
        }

        s_bounds_prev = s_bounds_curr;
    }
    else {
        s_bounds_prev = ChunkAlignedBox(ChunkPos::Constant(INT32_MIN), ChunkPos::Constant(INT32_MAX));
        s_bounds_curr = ChunkAlignedBox(ChunkPos::Zero(), ChunkPos::Zero());
        s_requests.clear();
    }
}
