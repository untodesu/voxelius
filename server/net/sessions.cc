#include "server/pch.hh"

#include "server/net/sessions.hh"

#include "core/config/map.hh"
#include "core/config/ref.hh"
#include "core/utils/crc64.hh"
#include "core/utils/epoch.hh"
#include "core/version.hh"

#include "shared/net/packet_auth.hh"
#include "shared/net/packet_session.hh"
#include "shared/net/protocol.hh"
#include "shared/world/biome_registry.hh"
#include "shared/world/block_registry.hh"
#include "shared/world/fluid_registry.hh"
#include "shared/world/tint_registry.hh"

#include "server/globals.hh"
#include "server/net/invites.hh"
#include "server/net/whitelist.hh"

static std::vector<Session> s_sessions;
static emhash8::HashMap<std::string, Session*> s_username_map;
static emhash8::HashMap<std::uint64_t, Session*> s_identity_map;

static config::Ref<bool> s_whitelist_enabled { false };
static config::Ref<bool> s_strict_version { false }; // allow differences in minor and patch versions
static config::Ref<unsigned> s_max_players { 8 };

static std::mt19937_64 s_randomizer;

std::uint16_t sessions::num_players = 0;

static bool is_outdated_client(std::uint32_t major, std::uint32_t minor, std::uint32_t patch)
{
    if(version::major > major) {
        return true;
    }

    if(s_strict_version) {
        if(version::minor > minor) {
            return true;
        }

        if(version::patch > patch) {
            return true;
        }
    }

    return false;
}

static bool is_outdated_server(std::uint32_t major, std::uint32_t minor, std::uint32_t patch)
{
    if(major > version::major) {
        return true;
    }

    if(s_strict_version) {
        if(minor > version::minor) {
            return true;
        }

        if(patch > version::patch) {
            return true;
        }
    }

    return false;
}

static void on_auth_request(const AuthRequest& packet)
{
    if(is_outdated_client(packet.major, packet.minor, packet.patch)) {
        Disconnect response {};
        response.reason = Disconnect::OUTDATED_CLIENT;
        protocol::send(response, packet.peer);
        return;
    }

    if(is_outdated_server(packet.major, packet.minor, packet.patch)) {
        Disconnect response {};
        response.reason = Disconnect::OUTDATED_SERVER;
        protocol::send(response, packet.peer);
        return;
    }

    if(packet.invite && s_whitelist_enabled) {
        if(!invites::consume(packet.invite, packet.pkey)) {
            Disconnect response {};
            response.reason = Disconnect::NOT_WHITELISTED;
            protocol::send(response, packet.peer);
            return;
        }
    }

    if(s_whitelist_enabled) {
        if(!whitelist::contains(packet.pkey)) {
            Disconnect response {};
            response.reason = Disconnect::NOT_WHITELISTED;
            protocol::send(response, packet.peer);
            return;
        }
    }

    auto checksums_match = true;
    checksums_match = checksums_match && packet.biomes_hash == biome_registry::checksum();
    checksums_match = checksums_match && packet.blocks_hash == block_registry::checksum();
    checksums_match = checksums_match && packet.fluids_hash == fluid_registry::checksum();
    checksums_match = checksums_match && packet.tints_hash == tint_registry::checksum();

    if(!checksums_match) {
        Disconnect response {};
        response.reason = Disconnect::CHECKSUM_MISMATCH;
        protocol::send(response, packet.peer);
        return;
    }

    auto session = sessions::create(packet.peer, packet.username);

    if(session == nullptr) {
        Disconnect response {};
        response.reason = Disconnect::SERVER_IS_FULL;
        protocol::send(response, packet.peer);
        return;
    }

    auto timestamp_sec = utils::epoch_seconds();
    auto timestamp_min = timestamp_sec / 60;
    auto timestamp_str = std::to_string(timestamp_min);
    auto distribution = std::uniform_int_distribution<std::uint64_t>(0, UINT8_MAX);

    for(std::size_t i = 0; i < session->nonce.size(); ++i) {
        session->nonce[i] = static_cast<std::byte>(distribution(s_randomizer));
    }

    session->salted_nonce.resize(session->nonce.size() + timestamp_str.size());
    std::memcpy(session->salted_nonce.data(), session->nonce.data(), session->nonce.size());
    std::memcpy(session->salted_nonce.data() + session->nonce.size(), timestamp_str.data(), timestamp_str.size());

    session->identity = utils::crc64(session->pkey);
    session->state = session_state::CHALLENGE;

    AuthChallenge response {};
    response.nonce = session->nonce;
    protocol::send(response, packet.peer);
}

static void on_auth_response(const AuthResponse& packet)
{
    auto session = sessions::find(packet.peer);

    if(session == nullptr) {
        Disconnect response {};
        response.reason = Disconnect::UNSPECIFIED;
        protocol::send(response, packet.peer);
        return;
    }

    if(!ed25519::verify(session->pkey, session->salted_nonce, packet.signature)) {
        Disconnect response {};
        response.reason = Disconnect::INVALID_SIGNATURE;
        protocol::send(response, packet.peer);
        return;
    }

    session->state = session_state::CONNECTED;

    AuthAdmission response {};
    response.client_id = session->client_id;
    response.identity = session->identity;
    response.username = session->username;
    protocol::send(response, packet.peer);

    // UNDONE: add player entity to AuthAdmission packet...
    // AuthAdmission should be a marker for the client to start
    // requesting chunks and the server to start sending chunks
    // in response to the client, or just send a bunch of component
    // update packets for the player entity idk
}

static void on_disconnect(const Disconnect& packet)
{
    if(auto session = sessions::find(packet.peer)) {
        LOG_INFO("{} ({}) disconnected: {}", session->username, session->identity, Disconnect::reason_string_server(packet.reason));

        sessions::destroy(session);
    }
}

static std::string sanitize_username(std::string_view username)
{
    auto size = username.size();
    size = std::min(size, static_cast<std::size_t>(64));

    std::string sanitized;
    sanitized.reserve(size);

    for(std::size_t i = 0; i < size; ++i) {
        auto character = username[i];
        auto is_allowed = false;

        if(i > 0) {
            is_allowed = is_allowed || std::isalnum(character);
            is_allowed = is_allowed || character == '_';
        }
        else {
            is_allowed = is_allowed || std::isalpha(character);
        }

        if(is_allowed) {
            sanitized.push_back(character);
        }
        else {
            sanitized.push_back('X');
        }
    }

    auto appender = 1;
    auto original = sanitized.substr(0, 60);

    while(s_username_map.contains(sanitized)) {
        sanitized = std::format("{}{}", original, appender);
        appender += 1;
    }

    return sanitized;
}

void sessions::init(void)
{
    s_whitelist_enabled.bind(globals::server_config, "whitelist.enabled");
    s_strict_version.bind(globals::server_config, "auth.strict_version");
    s_max_players.bind(globals::server_config, "host.max_players");

    std::random_device noise;
    s_randomizer.seed(noise());

    globals::dispatcher.sink<AuthRequest>().connect<&on_auth_request>();
    globals::dispatcher.sink<AuthResponse>().connect<&on_auth_response>();
    globals::dispatcher.sink<Disconnect>().connect<&on_disconnect>();
}

void sessions::init_late(void)
{
    auto max_players = s_max_players.value();
    max_players = std::max(max_players, 1U);
    max_players = std::min(max_players, 128U);

    // Propagate the change to the rest of
    // config::Ref instances bound to this slot
    s_max_players.set_value(max_players);

    s_username_map.clear();
    s_identity_map.clear();
    s_sessions.resize(max_players);

    for(std::size_t i = 0; i < max_players; ++i) {
        auto& session = s_sessions[i];
        session.state = session_state::UNCONNECTED;
        session.client_id = UINT16_MAX;
        session.identity = UINT64_MAX;
        session.username = std::string {};
        session.player = entt::null;
        session.peer = nullptr;
    }
}

void sessions::shutdown(void)
{
    s_username_map.clear();
    s_identity_map.clear();
    s_sessions.clear();
}

Session* sessions::create(ENetPeer* peer, std::string_view username)
{
    for(std::size_t i = 0; i < s_sessions.size(); ++i) {
        auto& session = s_sessions[i];

        if(session.peer == nullptr) {
            session.state = session_state::UNCONNECTED;
            session.client_id = static_cast<std::uint16_t>(i);
            session.identity = UINT64_MAX;
            session.username = sanitize_username(username);
            session.player = entt::null;
            session.peer = peer;

            s_username_map.insert_or_assign(session.username, &session);
            s_identity_map.insert_or_assign(session.identity, &session);

            peer->data = &session;

            num_players += 1;

            return &session;
        }
    }

    return nullptr;
}

Session* sessions::find(std::string_view username)
{
    auto key = std::string(username);
    auto it = s_username_map.find(key);

    if(it == s_username_map.cend()) {
        return nullptr;
    }

    return it->second;
}

Session* sessions::find(std::uint16_t client_id)
{
    if(client_id < s_sessions.size()) {
        auto& session = s_sessions[client_id];

        if(session.peer == nullptr || session.state == session_state::UNCONNECTED) {
            return nullptr;
        }

        return &session;
    }

    return nullptr;
}

Session* sessions::find(std::uint64_t identity)
{
    auto it = s_identity_map.find(identity);

    if(it == s_identity_map.cend()) {
        return nullptr;
    }

    return it->second;
}

Session* sessions::find(const ENetPeer* peer)
{
    if(peer == nullptr) {
        return nullptr;
    }

    auto session = reinterpret_cast<Session*>(peer->data);

    if(session == nullptr || session->state == session_state::UNCONNECTED) {
        return nullptr;
    }

    return session;
}

void sessions::destroy(Session* session)
{
    if(session) {
        if(session->peer) {
            session->peer->data = nullptr;
        }

        s_username_map.erase(session->username);
        s_identity_map.erase(session->identity);

        session->state = session_state::UNCONNECTED;
        session->client_id = UINT16_MAX;
        session->identity = UINT64_MAX;
        session->username = std::string {};
        session->player = entt::null;
        session->peer = nullptr;

        num_players -= 1;
    }
}
