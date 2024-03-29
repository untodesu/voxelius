// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2024, Voxelius Contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <shared/entity/chunk.hh>
#include <shared/event/chunk_create.hh>
#include <shared/event/chunk_remove.hh>
#include <shared/event/chunk_update.hh>
#include <shared/globals.hh>
#include <shared/world.hh>
#include <unordered_map>

// FIXME: speed! The standard hashmap implementation is
// comically slow, use a faster hashmap implementation!
// The probable candidate is https://github.com/ktprime/emhash
std::unordered_map<chunk_pos_t, Chunk *> map {};

Chunk *world::create_chunk(const chunk_pos_t &cpos)
{
    Chunk *chunk = world::find_chunk(cpos);

    if(chunk == nullptr) {
        chunk = new Chunk{};
        chunk->entity = globals::registry.create();
        chunk->voxels.fill(NULL_VOXEL);

        auto &comp = globals::registry.emplace<ChunkComponent>(chunk->entity);
        comp.chunk = chunk;
        comp.cpos = cpos;

        map.emplace(cpos, chunk);

        ChunkCreateEvent event = {};
        event.chunk = chunk;
        event.cpos = cpos;
        globals::dispatcher.trigger(event);
    }

    return chunk;
}

Chunk *world::find_chunk(const chunk_pos_t &cpos)
{
    if(const auto it = map.find(cpos); it != map.cend())
        return it->second;
    return nullptr;
}

void world::remove_chunk(const chunk_pos_t &cpos)
{
    if(const auto it = map.find(cpos); it != map.cend()) {
        ChunkCreateEvent event = {};
        event.chunk = it->second;
        event.cpos = cpos;
        globals::dispatcher.trigger(event);

        globals::registry.destroy(it->second->entity);

        delete it->second;

        map.erase(it);
    }
}

void world::remove_all_chunks()
{
    for(auto it = map.begin(); it != map.end();) {
        ChunkCreateEvent event = {};
        event.chunk = it->second;
        event.cpos = it->first;
        globals::dispatcher.trigger(event);

        globals::registry.destroy(it->second->entity);

        delete it->second;

        it = map.erase(it);
    }

    map.clear();
}

voxel_t world::get_voxel(const voxel_pos_t &vpos)
{
    const auto cpos = coord::to_chunk(vpos);
    const auto lpos = coord::to_local(vpos);
    const auto index = coord::to_index(lpos);
    if(const auto it = map.find(cpos); it != map.cend())
        return it->second->voxels[index];
    return NULL_VOXEL;
}

voxel_t world::get_voxel(const chunk_pos_t &cpos, const local_pos_t &lpos)
{
    const auto p_vpos = coord::to_voxel(cpos, lpos);
    const auto p_cpos = coord::to_chunk(p_vpos);
    const auto p_lpos = coord::to_local(p_vpos);
    const auto index = coord::to_index(p_lpos);
    if(const auto it = map.find(p_cpos); it != map.cend())
        return it->second->voxels[index];
    return NULL_VOXEL;
}

void world::set_voxel(const voxel_pos_t &vpos, voxel_t voxel)
{
    const auto cpos = coord::to_chunk(vpos);
    const auto lpos = coord::to_local(vpos);
    const auto index = coord::to_index(lpos);

    Chunk *chunk = world::create_chunk(cpos);
    chunk->voxels[index] = voxel;

    ChunkUpdateEvent event = {};
    event.chunk = chunk;
    event.voxel = voxel;
    event.cpos = cpos;
    event.lpos = lpos;
    event.vpos = vpos;
    event.index = index;
    globals::dispatcher.trigger(event);
}

void world::set_voxel(const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t voxel)
{
    const auto p_vpos = coord::to_voxel(cpos, lpos);
    const auto p_cpos = coord::to_chunk(p_vpos);
    const auto p_lpos = coord::to_local(p_vpos);
    const auto index = coord::to_index(p_lpos);

    Chunk *chunk = world::create_chunk(p_cpos);
    chunk->voxels[index] = voxel;

    ChunkUpdateEvent event = {};
    event.chunk = chunk;
    event.voxel = voxel;
    event.cpos = p_cpos;
    event.lpos = p_lpos;
    event.vpos = p_vpos;
    event.index = index;
    globals::dispatcher.trigger(event);
}
