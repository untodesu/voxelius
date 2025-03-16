#include "server/pch.hh"
#include "server/receive.hh"

#include "core/config.hh"

#include "shared/chunk_aabb.hh"
#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/head.hh"
#include "shared/protocol.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"

#include "server/game.hh"
#include "server/globals.hh"
#include "server/inhabited.hh"
#include "server/sessions.hh"
#include "server/universe.hh"
#include "server/worldgen.hh"

static void on_entity_transform_packet(const protocol::EntityTransform &packet)
{
    if(auto session = sessions::find(packet.peer)) {
        if(session->dimension && session->dimension->entities.valid(session->player_entity)) {
            auto &component = session->dimension->entities.emplace_or_replace<TransformComponent>(session->player_entity);
            component.angles = packet.angles;
            component.chunk = packet.chunk;
            component.local = packet.local;

            protocol::EntityTransform response;
            response.entity = session->player_entity;
            response.angles = component.angles;
            response.chunk = component.chunk;
            response.local = component.local;

            // Propagate changes to the rest of the world
            // except the peer that has sent the packet in the first place
            sessions::broadcast(session->dimension, protocol::encode(response), session->peer);
        }
    }
}

static void on_entity_velocity_packet(const protocol::EntityVelocity &packet)
{
    if(auto session = sessions::find(packet.peer)) {
        if(session->dimension && session->dimension->entities.valid(session->player_entity)) {
            auto &component = session->dimension->entities.emplace_or_replace<VelocityComponent>(session->player_entity);
            component.value = packet.value;

            protocol::EntityVelocity response;
            response.entity = session->player_entity;
            response.value = component.value;

            // Propagate changes to the rest of the world
            // except the peer that has sent the packet in the first place
            sessions::broadcast(session->dimension, protocol::encode(response), session->peer);
        }
    }
}

static void on_entity_head_packet(const protocol::EntityHead &packet)
{
    if(auto session = sessions::find(packet.peer)) {
        if(session->dimension && session->dimension->entities.valid(session->player_entity)) {
            auto &component = session->dimension->entities.emplace_or_replace<HeadComponent>(session->player_entity);
            component.angles = packet.angles;

            protocol::EntityHead response;
            response.entity = session->player_entity;
            response.angles = component.angles;

            // Propagate changes to the rest of the world
            // except the peer that has sent the packet in the first place
            sessions::broadcast(session->dimension, protocol::encode(response), session->peer);
        }
    }
}

static void on_set_voxel_packet(const protocol::SetVoxel &packet)
{
    if(auto session = sessions::find(packet.peer)) {
        if(session->dimension && !session->dimension->set_voxel(packet.voxel, packet.vpos)) {
            auto cpos = coord::to_chunk(packet.vpos);
            auto lpos = coord::to_local(packet.vpos);
            auto index = coord::to_index(lpos);

            if(worldgen::is_generating(session->dimension, cpos)) {
                // The chunk is currently being generated;
                // ignore all requests from players to build there
                return;
            }

            auto chunk = session->dimension->find_chunk(cpos);

            if(chunk == nullptr) {
                // The chunk is not loaded, so we must
                // ignore any requests from players to build there
                return;
            }

            chunk->set_voxel(packet.voxel, index);

            session->dimension->chunks.emplace_or_replace<InhabitedComponent>(chunk->get_entity());

            protocol::SetVoxel response;
            response.vpos = packet.vpos;
            response.voxel = packet.voxel;
            sessions::broadcast(session->dimension, protocol::encode(response), session->peer);

            return;
        }
    }
}

static void on_request_chunk_packet(const protocol::RequestChunk &packet)
{
    if(auto session = sessions::find(packet.peer)) {
        if(!session->dimension || !session->dimension->entities.valid(session->player_entity)) {
            // De-spawned sessions cannot request
            // chunks from the server; that's cheating!!!
            return;
        }

        if(auto transform = session->dimension->entities.try_get<TransformComponent>(session->player_entity)) {
            ChunkAABB view_box;
            view_box.min = transform->chunk - static_cast<chunk_pos::value_type>(server_game::view_distance.get_value());
            view_box.max = transform->chunk + static_cast<chunk_pos::value_type>(server_game::view_distance.get_value());

            if(view_box.contains(packet.cpos)) {
                if(auto chunk = universe::load_chunk(session->dimension, packet.cpos)) {
                    protocol::ChunkVoxels response;
                    response.chunk = packet.cpos;
                    response.voxels = chunk->get_voxels();
                    protocol::send(packet.peer, protocol::encode(response));
                }
                else {
                    worldgen::request_chunk(session, packet.cpos);
                }
            }
        }
    }
}

static void on_entity_sound_packet(const protocol::EntitySound &packet)
{
    if(auto session = sessions::find(packet.peer)) {
        if(!session->dimension || !session->dimension->entities.valid(session->player_entity)) {
            // De-spawned sessions cannot play sounds
            return;
        }

        protocol::EntitySound response;
        response.entity = session->player_entity;
        response.sound = packet.sound;
        response.looping = packet.looping;
        response.pitch = packet.pitch;

        sessions::broadcast(session->dimension, protocol::encode(response), packet.peer);
    }
}

void server_recieve::init(void)
{
    globals::dispatcher.sink<protocol::EntityTransform>().connect<&on_entity_transform_packet>();
    globals::dispatcher.sink<protocol::EntityVelocity>().connect<&on_entity_velocity_packet>();
    globals::dispatcher.sink<protocol::EntityHead>().connect<&on_entity_head_packet>();
    globals::dispatcher.sink<protocol::SetVoxel>().connect<&on_set_voxel_packet>();
    globals::dispatcher.sink<protocol::RequestChunk>().connect<&on_request_chunk_packet>();
    globals::dispatcher.sink<protocol::EntitySound>().connect<&on_entity_sound_packet>();
}
