#include "client/pch.hh"

#include "client/net/session.hh"

#include "core/buffer.hh"
#include "core/utils/epoch.hh"
#include "core/utils/physfs.hh"
#include "core/version.hh"

#include "shared/entity/class_registry.hh"
#include "shared/net/packet_auth.hh"
#include "shared/net/packet_session.hh"
#include "shared/net/protocol.hh"
#include "shared/world/biome_map.hh"
#include "shared/world/biome_registry.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/fluid_registry.hh"
#include "shared/world/tint_registry.hh"
#include "shared/world/world.hh"

#include "client/globals.hh"
#include "client/net/host.hh"

constexpr static std::string_view KEYPAIR_PATH = "keypair.dat";

session_state session::state;
std::uint64_t session::identity;
std::uint16_t session::client_id;
std::string session::assigned_username;

static ed25519::pair_type s_auth_pair;
static std::uint64_t s_pending_invite;

static void set_state(session_state state)
{
    session::state = state;

    globals::dispatcher.trigger(SessionStateEvent(state, 0));
}

static void reset_session(void)
{
    session::state = SESSION_DISCONNECTED;
    session::identity = UINT64_MAX;
    session::client_id = UINT16_MAX;
    session::assigned_username.clear();

    s_pending_invite = 0;
}

static void handle_disconnect(std::uint32_t reason)
{
    if(session::state) {
        reset_session();

        host::disconnect();

        globals::dispatcher.trigger(SessionStateEvent(SESSION_DISCONNECTED, reason));
    }

    globals::registry.clear();
    globals::player = entt::null;
    biome_map::purge();
    world::purge();
}

static void on_host_connect(const HostConnectEvent& event)
{
    set_state(SESSION_AUTHENTICATING);

    packet::Auth_Request request {};
    request.version_major = version::major;
    request.version_minor = version::minor;
    request.version_patch = version::patch;
    request.pkey = s_auth_pair.first;
    request.invite_code = s_pending_invite;
    request.biomes_hash = biome_registry::checksum();
    request.blocks_hash = block_registry::checksum();
    request.fluids_hash = fluid_registry::checksum();
    request.tints_hash = tint_registry::checksum();
    request.ents_hash = class_registry::checksum();
    request.username = std::string("testplayer"); // TODO: username setting

    auto& enet_event = event.event();
    protocol::send(request, enet_event.peer);
}

static void on_host_disconnect(const HostDisconnectEvent& event)
{
    if(session::state) {
        globals::dispatcher.trigger(SessionErrorEvent(packet::Session_Disconnect::UNSPECIFIED));

        handle_disconnect(packet::Session_Disconnect::UNSPECIFIED);
    }
}

static void on_auth_challenge(const packet::Auth_Challenge& packet)
{
    auto timestamp_sec = utils::epoch_seconds();
    auto timestamp_min = timestamp_sec / 60;
    auto timestamp_str = std::to_string(timestamp_min);

    std::vector<std::byte> nonce;
    nonce.resize(packet.nonce.size() + timestamp_str.size());
    std::memcpy(nonce.data(), packet.nonce.data(), packet.nonce.size());
    std::memcpy(nonce.data() + packet.nonce.size(), timestamp_str.data(), timestamp_str.size());

    packet::Auth_Response response {};
    response.signature = ed25519::sign(s_auth_pair, nonce);
    protocol::send(response, packet.peer);
}

static void on_auth_admission(const packet::Auth_Admission& packet)
{
    session::identity = packet.identity;
    session::client_id = packet.client_id;
    session::assigned_username = packet.username;

    set_state(SESSION_SPAWNING);
}

static void on_session_disconnect(const packet::Session_Disconnect& packet)
{
    globals::dispatcher.trigger(SessionErrorEvent(packet.reason));

    handle_disconnect(packet.reason);
}

void session::init(void)
{
    std::vector<std::byte> source;

    if(utils::read_file(KEYPAIR_PATH, source)) {
        ReadBuffer buffer(source);
        buffer.read_bytes(s_auth_pair.first);
        buffer.read_bytes(s_auth_pair.second);
    }
    else {
        auto seed = ed25519::generate_seed();
        s_auth_pair = ed25519::generate_pair(seed);

        WriteBuffer buffer;
        buffer.write_bytes(s_auth_pair.first);
        buffer.write_bytes(s_auth_pair.second);

        if(auto file = buffer.to_file(KEYPAIR_PATH, false)) {
            PHYSFS_close(file);
        }
    }

    globals::dispatcher.sink<HostConnectEvent>().connect<&on_host_connect>();
    globals::dispatcher.sink<HostDisconnectEvent>().connect<&on_host_disconnect>();
    globals::dispatcher.sink<packet::Auth_Challenge>().connect<&on_auth_challenge>();
    globals::dispatcher.sink<packet::Auth_Admission>().connect<&on_auth_admission>();
    globals::dispatcher.sink<packet::Session_Disconnect>().connect<&on_session_disconnect>();
}

void session::shutdown(void)
{
    if(state) {
        disconnect(packet::Session_Disconnect::CLIENT_SHUTDOWN);
    }
}

void session::connect(std::string_view host, std::uint16_t port, std::uint64_t invite)
{
    if(state == SESSION_DISCONNECTED) {
        s_pending_invite = invite;

        if(host::connect(host, port)) {
            set_state(SESSION_CONNECTING);
        }
        else {
            globals::dispatcher.trigger(SessionErrorEvent(packet::Session_Disconnect::UNSPECIFIED));

            handle_disconnect(packet::Session_Disconnect::UNSPECIFIED);
        }
    }
}

void session::disconnect(std::uint32_t reason)
{
    if(state && globals::peer) {
        packet::Session_Disconnect packet {};
        packet.reason = reason;
        protocol::send(packet, globals::peer);
    }

    handle_disconnect(reason);
}

void session::notify_spawned(void)
{
    if(state == SESSION_SPAWNING) {
        set_state(SESSION_INGAME);
    }
}
