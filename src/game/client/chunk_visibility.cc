#include "client/pch.hh"

#include "client/chunk_visibility.hh"

#include "core/config.hh"
#include "core/vectors.hh"

#include "shared/chunk.hh"
#include "shared/chunk_aabb.hh"
#include "shared/dimension.hh"
#include "shared/protocol.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/session.hh"

// Sending a somewhat large amount of network packets
// can easily overwhelm both client, server and the network
// channel created between the two. To prevent this from happening
// we throttle the client's ever increasing itch for new chunks
constexpr static unsigned int MAX_CHUNKS_REQUESTS_PER_FRAME = 16U;

static ChunkAABB current_view_box;
static ChunkAABB previous_view_box;
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
        auto da = vx::distance2(cpos_a, camera::position_chunk);
        auto db = vx::distance2(cpos_b, camera::position_chunk);
        return da > db;
    });
}

void chunk_visibility::update_late(void)
{
    current_view_box.min = camera::position_chunk - static_cast<chunk_pos::value_type>(camera::view_distance.get_value());
    current_view_box.max = camera::position_chunk + static_cast<chunk_pos::value_type>(camera::view_distance.get_value());

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
