#include "server/pch.hh"
#include "server/unloader.hh"

#include "core/config.hh"

#include "shared/chunk_aabb.hh"
#include "shared/chunk.hh"
#include "shared/dimension.hh"
#include "shared/player.hh"
#include "shared/player.hh"
#include "shared/transform.hh"

#include "server/game.hh"
#include "server/globals.hh"
#include "server/inhabited.hh"
#include "server/universe.hh"

static void on_chunk_update(const ChunkUpdateEvent &event)
{
    event.dimension->chunks.emplace_or_replace<InhabitedComponent>(event.chunk->get_entity());
}

static void on_voxel_set(const VoxelSetEvent &event)
{
    event.dimension->chunks.emplace_or_replace<InhabitedComponent>(event.chunk->get_entity());
}

void unloader::init(void)
{
    globals::dispatcher.sink<ChunkUpdateEvent>().connect<&on_chunk_update>();
    globals::dispatcher.sink<VoxelSetEvent>().connect<&on_voxel_set>();
}

void unloader::init_late(void)
{

}

void unloader::fixed_update_late(Dimension *dimension)
{
    auto group = dimension->entities.group(entt::get<PlayerComponent, TransformComponent>);
    auto boxes = std::vector<ChunkAABB>();

    for(const auto [entity, transform] : group.each()) {
        ChunkAABB aabb;
        aabb.min = transform.chunk - static_cast<chunk_pos::value_type>(server_game::view_distance.get_value());
        aabb.max = transform.chunk + static_cast<chunk_pos::value_type>(server_game::view_distance.get_value());
        boxes.push_back(aabb);
    }
    
    auto view = dimension->chunks.view<ChunkComponent>();
    auto chunk_in_view = false;

    for(const auto [entity, chunk] : view.each()) {
        chunk_in_view = false;

        for(const auto &aabb : boxes) {
            if(aabb.contains(chunk.cpos)) {
                chunk_in_view = true;
                break;
            }
        }

        if(chunk_in_view) {
            // The chunk is within view box of at least
            // a single player; we shouldn't unload it now
            continue;
        }

        if(dimension->chunks.any_of<InhabitedComponent>(entity)) {
            // Only store inhabited chunks on disk
            universe::save_chunk(dimension, chunk.cpos);
        }

        dimension->remove_chunk(entity);
    }
}
