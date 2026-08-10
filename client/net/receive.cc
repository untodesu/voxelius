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

static void on_entity_spawn(const packet::Entity_Spawn& packet)
{
    auto entity = utils::spawn(packet.class_id, packet.entity);

    if(!globals::registry.valid(entity)) {
        session::disconnect(packet::Session_Disconnect::ENTITY_ID_DESYNC);
        return;
    }

    if(entity != packet.entity) {
        session::disconnect(packet::Session_Disconnect::ENTITY_ID_DESYNC);
        return;
    }
}

static void on_entity_patch(const packet::Entity_Patch& packet)
{
    static ReadBuffer buffer;

    for(auto& it : packet.components) {
        buffer.reset(it.data);

        component_map::decode_net(it.id, packet.entity, buffer);
    }
}

static void on_entity_remove(const packet::Entity_Remove& packet)
{
    globals::registry.destroy(packet.entity);

    if(globals::player == packet.entity) {
        globals::player = entt::null;

        session::disconnect(packet::Session_Disconnect::ENTITY_ID_DESYNC); // We can't really recover from this
    }
}

static void on_entity_client(const packet::Entity_Client& packet)
{
    globals::player = packet.entity;

    session::notify_spawned();
}

static void on_world_blocks(const packet::World_Blocks& packet)
{
    auto chunk = world::create_chunk(packet.cpos);

    chunk->set_blocks(packet.blocks);

    globals::dispatcher.trigger(ChunkUpdateEvent(packet.cpos, chunk));
}

static void on_world_biomes(const packet::World_Biomes& packet)
{
    biome_map::insert(packet.realm, packet.cpos, packet.biomes);
}

static void on_world_set_block(const packet::World_SetBlock& packet)
{
    world::set_block(packet.bpos, packet.block);
}

static void on_world_timings(const packet::World_Timings& packet)
{
    globals::current_tick = packet.current_tick;
}

static void on_session_state(const SessionStateEvent& event)
{
    if(SESSION_DISCONNECTED == event.state()) {
        globals::player = entt::null;
    }
}

void receive::init(void)
{
    globals::dispatcher.sink<packet::Entity_Spawn>().connect<&on_entity_spawn>();
    globals::dispatcher.sink<packet::Entity_Patch>().connect<&on_entity_patch>();
    globals::dispatcher.sink<packet::Entity_Remove>().connect<&on_entity_remove>();
    globals::dispatcher.sink<packet::Entity_Client>().connect<&on_entity_client>();

    globals::dispatcher.sink<packet::World_Blocks>().connect<&on_world_blocks>();
    globals::dispatcher.sink<packet::World_Biomes>().connect<&on_world_biomes>();
    globals::dispatcher.sink<packet::World_SetBlock>().connect<&on_world_set_block>();
    globals::dispatcher.sink<packet::World_Timings>().connect<&on_world_timings>();

    globals::dispatcher.sink<SessionStateEvent>().connect<&on_session_state>();
}
