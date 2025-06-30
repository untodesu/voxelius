#include "client/pch.hh"

#include "client/world/chunk_visibility.hh"

#include "core/config/number.hh"
#include "core/math/vectors.hh"

#include "shared/world/chunk.hh"
#include "shared/world/chunk_aabb.hh"
#include "shared/world/dimension.hh"

#include "shared/protocol.hh"

#include "client/entity/camera.hh"

#include "client/globals.hh"
#include "client/session.hh"

// Sending a somewhat large amount of network packets
// can easily overwhelm both client, server and the network
// channel created between the two. To prevent this from happening
// we throttle the client's ever increasing itch for new chunks
constexpr static unsigned int MAX_CHUNKS_REQUESTS_PER_FRAME = 16U;

static world::ChunkAABB current_view_box;
static world::ChunkAABB previous_view_box;
static std::vector<chunk_pos> requests;

static void update_requests(void)
{
    requests.clear();

    for(auto cx = current_view_box.min.x; cx != current_view_box.max.x; cx += 1)
        for(auto cy = current_view_box.min.y; cy != current_view_box.max.y; cy += 1)
            for(auto cz = current_view_box.min.z; cz != current_view_box.max.z; cz += 1) {
                auto cpos = chunk_pos(cx, cy, cz);

                if(!globals::dimension->find_chunk(cpos)) {
                    requests.push_back(cpos);
                }
            }

    std::sort(requests.begin(), requests.end(), [](const chunk_pos& cpos_a, const chunk_pos& cpos_b) {
        auto da = math::distance2(cpos_a, entity::camera::position_chunk);
        auto db = math::distance2(cpos_b, entity::camera::position_chunk);
        return da > db;
    });
}

void world::chunk_visibility::update_late(void)
{
    current_view_box.min = entity::camera::position_chunk - static_cast<chunk_pos::value_type>(entity::camera::view_distance.get_value());
    current_view_box.max = entity::camera::position_chunk + static_cast<chunk_pos::value_type>(entity::camera::view_distance.get_value());

    if(!session::is_ingame()) {
        // This makes sure the previous view box
        // is always different from the current one
        previous_view_box.min = chunk_pos(INT32_MIN, INT32_MIN, INT32_MIN);
        previous_view_box.max = chunk_pos(INT32_MAX, INT32_MAX, INT32_MAX);
        return;
    }

    if((current_view_box.min != previous_view_box.min) || (current_view_box.max != previous_view_box.max)) {
        update_requests();
    }

    for(unsigned int i = 0U; i < MAX_CHUNKS_REQUESTS_PER_FRAME; ++i) {
        if(requests.empty()) {
            // Done sending requests
            break;
        }

        protocol::RequestChunk packet;
        packet.cpos = requests.back();
        protocol::send(session::peer, protocol::encode(packet));

        requests.pop_back();
    }

    auto view = globals::dimension->chunks.view<ChunkComponent>();

    for(const auto [entity, chunk] : view.each()) {
        if(!current_view_box.contains(chunk.cpos)) {
            globals::dimension->remove_chunk(entity);
        }
    }

    previous_view_box = current_view_box;
}
