#include "client/pch.hh"

#include "client/net/receive.hh"

#include "shared/entity/class_registry.hh"
#include "shared/entity/component_map.hh"
#include "shared/net/packet_entity.hh"
#include "shared/net/packet_session.hh"
#include "shared/net/packet_world.hh"
#include "shared/net/protocol.hh"
#include "shared/utils/entity.hh"
#include "shared/world/biome_map.hh"
#include "shared/world/world.hh"

#include "client/globals.hh"
#include "client/net/session.hh"

static void on_entity_spawn(const EntitySpawn_Packet& packet)
{
    auto entity = utils::spawn(packet.class_id, packet.entity);

    if(!globals::registry.valid(entity)) {
        session::disconnect(Disconnect_Packet::ENTITY_ID_DESYNC);
        return;
    }

    if(entity != packet.entity) {
        session::disconnect(Disconnect_Packet::ENTITY_ID_DESYNC);
        return;
    }
}

static void on_entity_patch(const EntityPatch_Packet& packet)
{
    static ReadBuffer buffer;

    for(auto& it : packet.components) {
        buffer.reset(it.data);

        component_map::decode_net(it.id, packet.entity, buffer);
    }
}

static void on_entity_remove(const EntityRemove_Packet& packet)
{
    globals::registry.destroy(packet.entity);

    if(globals::player == packet.entity) {
        globals::player = entt::null;

        session::disconnect(Disconnect_Packet::ENTITY_ID_DESYNC); // We can't really recover from this
    }
}

static void on_entity_client(const EntityClient_Packet& packet)
{
    globals::player = packet.entity;

    session::notify_spawned();
}

static void on_chunk_blocks(const ChunkBlocks_Packet& packet)
{
    auto chunk = world::create_chunk(packet.cpos);

    chunk->set_blocks(packet.blocks);

    globals::dispatcher.trigger(ChunkUpdateEvent(packet.cpos, chunk));
}

static void on_chunk_biomes(const ChunkBiomes_Packet& packet)
{
    biome_map::insert(packet.realm, packet.cpos, packet.biomes);
}

static void on_set_block(const SetBlock_Packet& packet)
{
    world::set_block(packet.bpos, packet.block);
}

static void on_session_state(const SessionStateEvent& event)
{
    if(SESSION_DISCONNECTED == event.state()) {
        globals::player = entt::null;
    }
}

void receive::init(void)
{
    globals::dispatcher.sink<EntitySpawn_Packet>().connect<&on_entity_spawn>();
    globals::dispatcher.sink<EntityPatch_Packet>().connect<&on_entity_patch>();
    globals::dispatcher.sink<EntityRemove_Packet>().connect<&on_entity_remove>();
    globals::dispatcher.sink<EntityClient_Packet>().connect<&on_entity_client>();

    globals::dispatcher.sink<ChunkBlocks_Packet>().connect<&on_chunk_blocks>();
    globals::dispatcher.sink<ChunkBiomes_Packet>().connect<&on_chunk_biomes>();
    globals::dispatcher.sink<SetBlock_Packet>().connect<&on_set_block>();

    globals::dispatcher.sink<SessionStateEvent>().connect<&on_session_state>();
}
