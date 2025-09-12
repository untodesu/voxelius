#include "client/pch.hh"

#include "client/session.hh"

#include "core/config/string.hh"

#include "core/math/crc64.hh"

#include "core/version.hh"

#include "shared/entity/head.hh"
#include "shared/entity/player.hh"
#include "shared/entity/transform.hh"
#include "shared/entity/velocity.hh"

#include "shared/world/dimension.hh"
#include "shared/world/item_registry.hh"
#include "shared/world/voxel_registry.hh"

#include "shared/coord.hh"
#include "shared/protocol.hh"

#include "client/entity/camera.hh"

#include "client/gui/chat.hh"
#include "client/gui/gui_screen.hh"
#include "client/gui/message_box.hh"
#include "client/gui/progress_bar.hh"
#include "client/gui/window_title.hh"

#include "client/world/chunk_visibility.hh"

#include "client/game.hh"
#include "client/globals.hh"

ENetPeer* session::peer = nullptr;
std::uint16_t session::client_index = UINT16_MAX;
std::uint64_t session::client_identity = UINT64_MAX;

static std::uint64_t server_password_hash = UINT64_MAX;

static void set_fixed_tickrate(std::uint16_t tickrate)
{
    globals::fixed_frametime_us = 1000000U / math::clamp<std::uint64_t>(tickrate, 10U, 300U);
    globals::fixed_frametime = static_cast<float>(globals::fixed_frametime_us) / 1000000.0f;
    globals::fixed_accumulator = 0;
}

static void on_login_response_packet(const protocol::LoginResponse& packet)
{
    spdlog::info("session: assigned client_index={}", packet.client_index);
    spdlog::info("session: assigned client_identity={}", packet.client_identity);
    spdlog::info("session: server ticks at {} TPS", packet.server_tickrate);

    session::client_index = packet.client_index;
    session::client_identity = packet.client_identity;

    set_fixed_tickrate(packet.server_tickrate);

    gui::progress_bar::set_title("connecting.loading_world");
}

static void on_disconnect_packet(const protocol::Disconnect& packet)
{
    enet_peer_disconnect(session::peer, 0);

    spdlog::info("session: disconnected: {}", packet.reason);

    gui::client_chat::clear();

    session::peer = nullptr;
    session::client_index = UINT16_MAX;
    session::client_identity = UINT64_MAX;

    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_frametime = 0.0f;
    globals::fixed_accumulator = 0;

    server_password_hash = UINT64_MAX;

    delete globals::dimension;
    globals::player = entt::null;
    globals::dimension = nullptr;

    gui::message_box::reset();
    gui::message_box::set_title("disconnected.disconnected");
    gui::message_box::set_subtitle(packet.reason.c_str());
    gui::message_box::add_button("disconnected.back", [](void) {
        globals::gui_screen = GUI_PLAY_MENU;
        gui::window_title::update();
    });

    globals::gui_screen = GUI_MESSAGE_BOX;
}

static void on_set_voxel_packet(const protocol::SetVoxel& packet)
{
    auto cpos = coord::to_chunk(packet.vpos);
    auto lpos = coord::to_local(packet.vpos);
    auto index = coord::to_index(lpos);

    if(auto chunk = globals::dimension->find_chunk(cpos)) {
        auto packet_voxel = world::voxel_registry::find(packet.voxel);

        if(chunk->get_voxel(index) != packet_voxel) {
            chunk->set_voxel(packet_voxel, index);

            world::ChunkUpdateEvent event;
            event.dimension = globals::dimension;
            event.chunk = chunk;
            event.cpos = cpos;

            // Send a generic ChunkUpdate event to shake
            // up the mesher; directly calling world::set_voxel
            // here would result in a networked feedback loop
            // caused by event handler below tripping
            globals::dispatcher.trigger(event);
        }
    }
}

// NOTE: [session] is a good place for this since [receive]
// handles entity data sent by the server and [session] handles
// everything else network related that is not player movement
static void on_voxel_set(const world::VoxelSetEvent& event)
{
    if(session::peer) {
        // Propagate changes to the server
        // FIXME: should we also validate things here or wait for the server to do so
        protocol::SetVoxel packet;
        packet.vpos = coord::to_voxel(event.cpos, event.lpos);
        packet.voxel = event.voxel ? event.voxel->get_id() : NULL_VOXEL_ID;

        protocol::send(session::peer, protocol::encode(packet));
    }
}

void session::init(void)
{
    session::peer = nullptr;
    session::client_index = UINT16_MAX;
    session::client_identity = UINT64_MAX;

    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_frametime = 0.0f;
    globals::fixed_accumulator = 0;

    server_password_hash = UINT64_MAX;

    globals::dispatcher.sink<protocol::LoginResponse>().connect<&on_login_response_packet>();
    globals::dispatcher.sink<protocol::Disconnect>().connect<&on_disconnect_packet>();
    globals::dispatcher.sink<protocol::SetVoxel>().connect<&on_set_voxel_packet>();

    globals::dispatcher.sink<world::VoxelSetEvent>().connect<&on_voxel_set>();
}

void session::shutdown(void)
{
    session::disconnect("protocol.client_shutdown");

    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_frametime = 0.0f;
    globals::fixed_accumulator = 0;
}

void session::invalidate(void)
{
    if(session::peer) {
        enet_peer_reset(session::peer);

        gui::message_box::reset();
        gui::message_box::set_title("disconnected.disconnected");
        gui::message_box::set_subtitle("enet.peer_connection_timeout");
        gui::message_box::add_button("disconnected.back", [](void) {
            globals::gui_screen = GUI_PLAY_MENU;
            gui::window_title::update();
        });

        globals::gui_screen = GUI_MESSAGE_BOX;
    }

    gui::client_chat::clear();

    session::peer = nullptr;
    session::client_index = UINT16_MAX;
    session::client_identity = UINT64_MAX;

    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_frametime = 0.0f;
    globals::fixed_accumulator = 0;

    server_password_hash = UINT64_MAX;

    delete globals::dimension;
    globals::player = entt::null;
    globals::dimension = nullptr;
}

void session::connect(std::string_view host, std::uint16_t port, std::string_view password)
{
    ENetAddress address;
    enet_address_set_host(&address, std::string(host).c_str());
    address.port = port;

    session::peer = enet_host_connect(globals::client_host, &address, 1, 0);
    session::client_index = UINT16_MAX;
    session::client_identity = UINT64_MAX;

    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_frametime = 0.0f;
    globals::fixed_accumulator = 0;

    server_password_hash = math::crc64(password.data(), password.size());

    if(!session::peer) {
        server_password_hash = UINT64_MAX;

        gui::message_box::reset();
        gui::message_box::set_title("disconnected.disconnected");
        gui::message_box::set_subtitle("enet.peer_connection_failed");
        gui::message_box::add_button("disconnected.back", [](void) {
            globals::gui_screen = GUI_PLAY_MENU;
            gui::window_title::update();
        });

        globals::gui_screen = GUI_MESSAGE_BOX;

        return;
    }

    gui::progress_bar::reset();
    gui::progress_bar::set_title("connecting.connecting");
    gui::progress_bar::set_button("connecting.cancel_button", [](void) {
        enet_peer_disconnect(session::peer, 0);

        session::peer = nullptr;
        session::client_index = UINT16_MAX;
        session::client_identity = UINT64_MAX;

        globals::fixed_frametime_us = UINT64_MAX;
        globals::fixed_frametime = 0.0f;
        globals::fixed_accumulator = 0;

        server_password_hash = UINT64_MAX;

        delete globals::dimension;
        globals::player = entt::null;
        globals::dimension = nullptr;

        globals::gui_screen = GUI_PLAY_MENU;
    });

    globals::gui_screen = GUI_PROGRESS_BAR;
}

void session::disconnect(std::string_view reason)
{
    if(session::peer) {
        protocol::Disconnect packet;
        packet.reason = std::string(reason);

        protocol::send(session::peer, protocol::encode(packet));

        enet_host_flush(globals::client_host);
        enet_host_service(globals::client_host, nullptr, 50);
        enet_peer_reset(session::peer);

        session::peer = nullptr;
        session::client_index = UINT16_MAX;
        session::client_identity = UINT64_MAX;

        globals::fixed_frametime_us = UINT64_MAX;
        globals::fixed_frametime = 0.0f;
        globals::fixed_accumulator = 0;

        server_password_hash = UINT64_MAX;

        delete globals::dimension;
        globals::player = entt::null;
        globals::dimension = nullptr;

        gui::client_chat::clear();
    }
}

void session::send_login_request(void)
{
    protocol::LoginRequest packet;
    packet.game_version_major = version::major;
    packet.voxel_registry_checksum = world::voxel_registry::get_checksum();
    packet.item_registry_checksum = world::item_registry::get_checksum();
    packet.password_hash = server_password_hash;
    packet.username = client_game::username.get();
    packet.game_version_minor = version::minor;
    packet.game_version_patch = version::patch;

    protocol::send(session::peer, protocol::encode(packet));

    server_password_hash = UINT64_MAX;

    gui::progress_bar::set_title("connecting.logging_in");
    globals::gui_screen = GUI_PROGRESS_BAR;
}

bool session::is_ingame(void)
{
    if(globals::dimension) {
        return globals::dimension->entities.valid(globals::player);
    }
    else {
        return false;
    }
}
