#include "server/pch.hh"

#include "server/world/unloader.hh"

#include "core/config/number.hh"

#include "shared/entity/player.hh"
#include "shared/entity/transform.hh"

#include "shared/world/chunk.hh"
#include "shared/world/chunk_aabb.hh"
#include "shared/world/dimension.hh"

#include "server/world/inhabited.hh"
#include "server/world/universe.hh"

#include "server/game.hh"
#include "server/globals.hh"

static void on_chunk_update(const world::ChunkUpdateEvent& event)
{
    event.dimension->chunks.emplace_or_replace<world::Inhabited>(event.chunk->get_entity());
}

static void on_voxel_set(const world::VoxelSetEvent& event)
{
    event.dimension->chunks.emplace_or_replace<world::Inhabited>(event.chunk->get_entity());
}

void world::unloader::init(void)
{
    globals::dispatcher.sink<world::ChunkUpdateEvent>().connect<&on_chunk_update>();
    globals::dispatcher.sink<world::VoxelSetEvent>().connect<&on_voxel_set>();
}

void world::unloader::init_late(void)
{
}

void world::unloader::fixed_update_late(Dimension* dimension)
{
    auto group = dimension->entities.group(entt::get<entity::Player, entity::Transform>);
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

        for(const auto& aabb : boxes) {
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

        if(dimension->chunks.any_of<Inhabited>(entity)) {
            // Only store inhabited chunks on disk
            world::universe::save_chunk(dimension, chunk.cpos);
        }

        dimension->remove_chunk(entity);
    }
}
