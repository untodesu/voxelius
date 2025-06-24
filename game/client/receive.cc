#include "client/pch.hh"

#include "client/receive.hh"

#include "shared/dimension.hh"
#include "shared/head.hh"
#include "shared/player.hh"
#include "shared/protocol.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"

#include "client/chat.hh"
#include "client/factory.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/message_box.hh"
#include "client/session.hh"
#include "client/sound.hh"
#include "client/window_title.hh"

static bool synchronize_entity_id(Dimension* dimension, entt::entity entity)
{
    if(dimension->entities.valid(entity)) {
        // Entity ID already exists
        return true;
    }

    auto created = dimension->entities.create(entity);

    if(created == entity) {
        // Synchronized successfully
        return true;
    }

    session::disconnect("protocol.entity_id_desync");
    spdlog::critical("receive: entity desync: network {} resolved as client {}", static_cast<std::uint64_t>(entity), static_cast<std::uint64_t>(created));

    message_box::reset();
    message_box::set_title("disconnected.disconnected");
    message_box::set_subtitle("protocol.entity_id_desync");
    message_box::add_button("disconnected.back", [](void) {
        globals::gui_screen = GUI_PLAY_MENU;
        window_title::update();
    });

    globals::gui_screen = GUI_MESSAGE_BOX;

    return false;
}

static void on_dimension_info_packet(const protocol::DimensionInfo& packet)
{
    if(session::peer) {
        if(globals::dimension) {
            delete globals::dimension;
            globals::dimension = nullptr;
            globals::player = entt::null;
        }

        globals::dimension = new Dimension(packet.name.c_str(), packet.gravity);
    }
}

static void on_chunk_voxels_packet(const protocol::ChunkVoxels& packet)
{
    if(session::peer && globals::dimension) {
        auto chunk = globals::dimension->create_chunk(packet.chunk);
        chunk->set_voxels(packet.voxels);

        ChunkUpdateEvent event;
        event.dimension = globals::dimension;
        event.cpos = packet.chunk;
        event.chunk = chunk;

        globals::dispatcher.trigger(event);

        return;
    }
}

static void on_entity_head_packet(const protocol::EntityHead& packet)
{
    if(session::peer && globals::dimension) {
        if(synchronize_entity_id(globals::dimension, packet.entity)) {
            auto& component = globals::dimension->entities.get_or_emplace<HeadComponent>(packet.entity);
            auto& prevcomp = globals::dimension->entities.get_or_emplace<HeadComponentPrev>(packet.entity);

            // Store the previous component state
            prevcomp.angles = component.angles;
            prevcomp.offset = component.offset;

            // Assign the new component state
            component.angles = packet.angles;
        }
    }
}

static void on_entity_transform_packet(const protocol::EntityTransform& packet)
{
    if(session::peer && globals::dimension) {
        if(synchronize_entity_id(globals::dimension, packet.entity)) {
            auto& component = globals::dimension->entities.get_or_emplace<TransformComponent>(packet.entity);
            auto& prevcomp = globals::dimension->entities.get_or_emplace<TransformComponentPrev>(packet.entity);

            // Store the previous component state
            prevcomp.angles = component.angles;
            prevcomp.chunk = component.chunk;
            prevcomp.local = component.local;

            // Assign the new component state
            component.angles = packet.angles;
            component.chunk = packet.chunk;
            component.local = packet.local;
        }
    }
}

static void on_entity_velocity_packet(const protocol::EntityVelocity& packet)
{
    if(session::peer && globals::dimension) {
        if(synchronize_entity_id(globals::dimension, packet.entity)) {
            auto& component = globals::dimension->entities.get_or_emplace<VelocityComponent>(packet.entity);
            component.value = packet.value;
        }
    }
}

static void on_entity_player_packet(const protocol::EntityPlayer& packet)
{
    if(session::peer && globals::dimension) {
        if(synchronize_entity_id(globals::dimension, packet.entity)) {
            client_factory::create_player(globals::dimension, packet.entity);
        }
    }
}

static void on_spawn_player_packet(const protocol::SpawnPlayer& packet)
{
    if(session::peer && globals::dimension) {
        if(synchronize_entity_id(globals::dimension, packet.entity)) {
            client_factory::create_player(globals::dimension, packet.entity);

            globals::player = packet.entity;
            globals::gui_screen = GUI_SCREEN_NONE;

            client_chat::refresh_timings();

            window_title::update();
        }
    }
}

static void on_remove_entity_packet(const protocol::RemoveEntity& packet)
{
    if(globals::dimension) {
        if(packet.entity == globals::player) {
            globals::player = entt::null;
        }

        globals::dimension->entities.destroy(packet.entity);
    }
}

static void on_generic_sound_packet(const protocol::GenericSound& packet)
{
    sound::play_generic(packet.sound.c_str(), packet.looping, packet.pitch);
}

static void on_entity_sound_packet(const protocol::EntitySound& packet)
{
    sound::play_entity(packet.entity, packet.sound.c_str(), packet.looping, packet.pitch);
}

void client_receive::init(void)
{
    globals::dispatcher.sink<protocol::DimensionInfo>().connect<&on_dimension_info_packet>();
    globals::dispatcher.sink<protocol::ChunkVoxels>().connect<&on_chunk_voxels_packet>();
    globals::dispatcher.sink<protocol::EntityHead>().connect<&on_entity_head_packet>();
    globals::dispatcher.sink<protocol::EntityTransform>().connect<&on_entity_transform_packet>();
    globals::dispatcher.sink<protocol::EntityVelocity>().connect<&on_entity_velocity_packet>();
    globals::dispatcher.sink<protocol::EntityPlayer>().connect<&on_entity_player_packet>();
    globals::dispatcher.sink<protocol::SpawnPlayer>().connect<&on_spawn_player_packet>();
    globals::dispatcher.sink<protocol::RemoveEntity>().connect<&on_remove_entity_packet>();
    globals::dispatcher.sink<protocol::GenericSound>().connect<&on_generic_sound_packet>();
    globals::dispatcher.sink<protocol::EntitySound>().connect<&on_entity_sound_packet>();
}
