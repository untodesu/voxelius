#include "client/pch.hh"

#include "client/session.hh"

#include "core/config.hh"
#include "core/crc64.hh"

#include "shared/coord.hh"
#include "shared/dimension.hh"
#include "shared/head.hh"
#include "shared/item_registry.hh"
#include "shared/player.hh"
#include "shared/protocol.hh"
#include "shared/transform.hh"
#include "shared/velocity.hh"
#include "shared/voxel_registry.hh"

#include "client/camera.hh"
#include "client/chat.hh"
#include "client/chunk_visibility.hh"
#include "client/game.hh"
#include "client/globals.hh"
#include "client/gui_screen.hh"
#include "client/message_box.hh"
#include "client/progress_bar.hh"
#include "client/window_title.hh"

ENetPeer* session::peer = nullptr;
std::uint16_t session::client_index = UINT16_MAX;
std::uint64_t session::client_identity = UINT64_MAX;

static std::uint64_t server_password_hash = UINT64_MAX;

static void set_fixed_tickrate(std::uint16_t tickrate)
{
    globals::fixed_frametime_us = 1000000U / cxpr::clamp<std::uint64_t>(tickrate, 10U, 300U);
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

    progress_bar::set_title("connecting.loading_world");
}

static void on_disconnect_packet(const protocol::Disconnect& packet)
{
    enet_peer_disconnect(session::peer, 0);

    spdlog::info("session: disconnected: {}", packet.reason);

    client_chat::clear();

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

    message_box::reset();
    message_box::set_title("disconnected.disconnected");
    message_box::set_subtitle(packet.reason.c_str());
    message_box::add_button("disconnected.back", [](void) {
        globals::gui_screen = GUI_PLAY_MENU;
        window_title::update();
    });

    globals::gui_screen = GUI_MESSAGE_BOX;
}

static void on_set_voxel_packet(const protocol::SetVoxel& packet)
{
    auto cpos = coord::to_chunk(packet.vpos);
    auto lpos = coord::to_local(packet.vpos);
    auto index = coord::to_index(lpos);

    if(auto chunk = globals::dimension->find_chunk(cpos)) {
        if(chunk->get_voxel(index) != packet.voxel) {
            chunk->set_voxel(packet.voxel, index);

            ChunkUpdateEvent event;
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
static void on_voxel_set(const VoxelSetEvent& event)
{
    if(session::peer) {
        // Propagate changes to the server
        // FIXME: should we also validate things here or wait for the server to do so
        protocol::SetVoxel packet;
        packet.vpos = coord::to_voxel(event.cpos, event.lpos);
        packet.voxel = event.voxel;

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

    globals::dispatcher.sink<VoxelSetEvent>().connect<&on_voxel_set>();
}

void session::deinit(void)
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

        message_box::reset();
        message_box::set_title("disconnected.disconnected");
        message_box::set_subtitle("enet.peer_connection_timeout");
        message_box::add_button("disconnected.back", [](void) {
            globals::gui_screen = GUI_PLAY_MENU;
            window_title::update();
        });

        globals::gui_screen = GUI_MESSAGE_BOX;
    }

    client_chat::clear();

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

void session::connect(const char* host, std::uint16_t port, const char* password)
{
    ENetAddress address;
    enet_address_set_host(&address, host);
    address.port = port;

    session::peer = enet_host_connect(globals::client_host, &address, 1, 0);
    session::client_index = UINT16_MAX;
    session::client_identity = UINT64_MAX;

    globals::fixed_frametime_us = UINT64_MAX;
    globals::fixed_frametime = 0.0f;
    globals::fixed_accumulator = 0;

    server_password_hash = crc64::get(password);

    if(!session::peer) {
        server_password_hash = UINT64_MAX;

        message_box::reset();
        message_box::set_title("disconnected.disconnected");
        message_box::set_subtitle("enet.peer_connection_failed");
        message_box::add_button("disconnected.back", [](void) {
            globals::gui_screen = GUI_PLAY_MENU;
            window_title::update();
        });

        globals::gui_screen = GUI_MESSAGE_BOX;

        return;
    }

    progress_bar::reset();
    progress_bar::set_title("connecting.connecting");
    progress_bar::set_button("connecting.cancel_button", [](void) {
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

void session::disconnect(const char* reason)
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

        client_chat::clear();
    }
}

void session::send_login_request(void)
{
    protocol::LoginRequest packet;
    packet.version = protocol::VERSION;
    packet.voxel_registry_checksum = voxel_registry::calcualte_checksum();
    packet.item_registry_checksum = item_registry::calcualte_checksum();
    packet.password_hash = server_password_hash;
    packet.username = client_game::username.get();

    protocol::send(session::peer, protocol::encode(packet));

    server_password_hash = UINT64_MAX;

    progress_bar::set_title("connecting.logging_in");
    globals::gui_screen = GUI_PROGRESS_BAR;
}

bool session::is_ingame(void)
{
    if(globals::dimension) {
        return globals::dimension->entities.valid(globals::player);
    } else {
        return false;
    }
}
