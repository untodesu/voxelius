#include "client/pch.hh"
#include "client/chunk_visibility.hh"

#include "core/config.hh"

#include "shared/chunk.hh"
#include "shared/dimension.hh"
#include "shared/protocol.hh"

#include "client/camera.hh"
#include "client/globals.hh"
#include "client/session.hh"

static chunk_pos cached_cpos;
static unsigned int cached_dist;
static std::vector<chunk_pos> requests;

static void request_chunk(const chunk_pos &cpos)
{
    protocol::RequestChunk packet;
    packet.cpos = cpos;
    protocol::send(session::peer, protocol::encode(packet));
}

// Go through the list of chunk positions that should
// be visible client-side but seem to not exist yet
static void request_new_chunks(void)
{
    auto cmin = cached_cpos - static_cast<chunk_pos::value_type>(cached_dist);
    auto cmax = cached_cpos + static_cast<chunk_pos::value_type>(cached_dist);

    requests.clear();

    for(auto cx = cmin.x; cx <= cmax.x; ++cx)
    for(auto cy = cmin.y; cy <= cmax.y; ++cy)
    for(auto cz = cmin.z; cz <= cmax.z; ++cz) {
        if(globals::dimension->find_chunk({cx, cy, cz})) {
            // The chunk already exists, we don't need
            // to request it from the server anymore
            continue;
        }

        requests.push_back(chunk_pos(cx, cy, cz));
    }

    std::sort(requests.begin(), requests.end(), [](const chunk_pos &ca, const chunk_pos &cb) {
        auto dir_a = ca - cached_cpos;
        auto dir_b = cb - cached_cpos;

        const auto da = dir_a[0] * dir_a[0] + dir_a[1] * dir_a[1] + dir_a[2] * dir_a[2];
        const auto db = dir_b[0] * dir_b[0] + dir_b[1] * dir_b[1] + dir_b[2] * dir_b[2];

        return da > db;
    });
}

static bool is_chunk_visible(const chunk_pos &cpos)
{
    const auto dx = cxpr::abs(cpos.x - cached_cpos.x);
    const auto dy = cxpr::abs(cpos.z - cached_cpos.z);

    if((dx <= cached_dist) && (dy <= cached_dist))
        return true;
    return false;
}

void chunk_visibility::update_chunk(entt::entity entity)
{
    if(auto component = globals::dimension->chunks.try_get<ChunkComponent>(entity)) {
        if(is_chunk_visible(component->cpos))
            globals::dimension->chunks.emplace_or_replace<ChunkVisibleComponent>(entity);
        else globals::dimension->chunks.remove<ChunkVisibleComponent>(entity);
    }
}

void chunk_visibility::update_chunk(const chunk_pos &cpos)
{
    if(auto chunk = globals::dimension->find_chunk(cpos)) {
        const auto &component = globals::dimension->chunks.get<ChunkComponent>(chunk->get_entity());
        if(is_chunk_visible(component.cpos))
            globals::dimension->chunks.emplace_or_replace<ChunkVisibleComponent>(chunk->get_entity());
        else globals::dimension->chunks.remove<ChunkVisibleComponent>(chunk->get_entity());
    }
}

void chunk_visibility::update_chunks(void)
{
    const auto view = globals::dimension->chunks.view<ChunkComponent>();
    
    for(const auto [entity, chunk] : view.each()) {
        if(is_chunk_visible(chunk.cpos))
            globals::dimension->chunks.emplace_or_replace<ChunkVisibleComponent>(entity);
        else globals::dimension->chunks.remove<ChunkVisibleComponent>(entity);
    }

    request_new_chunks();
}

void chunk_visibility::cleanup(void)
{
    cached_cpos = camera::position_chunk + 1;
    cached_dist = camera::view_distance.get_value() + 1;
    requests.clear();
}

void chunk_visibility::update(void)
{
    if(session::is_ingame()) {
        if((cached_cpos != camera::position_chunk) || (cached_dist != camera::view_distance.get_value())) {
            cached_cpos = camera::position_chunk;
            cached_dist = camera::view_distance.get_value();
            chunk_visibility::update_chunks();
            return;
        }

        for(int i = 0; i < 16; ++i) {
            if(requests.empty()) {
                // Done sending requests
                break;
            }

            request_chunk(requests.back());

            requests.pop_back();
        }
        
        cached_cpos = camera::position_chunk;
        cached_dist = camera::view_distance.get_value();
        return;
    }
    
    cached_cpos = camera::position_chunk + 1;
    cached_dist = camera::view_distance.get_value() + 1;
}
