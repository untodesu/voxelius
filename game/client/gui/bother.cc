#include "client/pch.hh"

#include "client/gui/bother.hh"

#include "core/version.hh"

#include "shared/protocol.hh"

#include "client/globals.hh"

// Maximum amount of peers used for bothering
constexpr static std::size_t BOTHER_PEERS = 4;

struct BotherQueueItem final {
    unsigned int identity;
    std::string hostname;
    std::uint16_t port;
};

static ENetHost* bother_host;
static entt::dispatcher bother_dispatcher;
static std::unordered_set<unsigned int> bother_set;
static std::deque<BotherQueueItem> bother_queue;

static void on_status_response_packet(const protocol::StatusResponse& packet)
{
    auto identity = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(packet.peer->data));

    bother_set.erase(identity);

    gui::BotherResponseEvent event;
    event.identity = identity;
    event.is_server_unreachable = false;
    event.num_players = packet.num_players;
    event.max_players = packet.max_players;
    event.motd = packet.motd;
    event.game_version_major = packet.game_version_major;
    event.game_version_minor = packet.game_version_minor;
    event.game_version_patch = packet.game_version_patch;
    globals::dispatcher.trigger(event);

    enet_peer_disconnect(packet.peer, protocol::CHANNEL);
}

void gui::bother::init(void)
{
    bother_host = enet_host_create(nullptr, BOTHER_PEERS, 1, 0, 0);
    bother_dispatcher.clear();
    bother_set.clear();

    bother_dispatcher.sink<protocol::StatusResponse>().connect<&on_status_response_packet>();
}

void gui::bother::shutdown(void)
{
    enet_host_destroy(bother_host);
    bother_dispatcher.clear();
    bother_set.clear();
}

void gui::bother::update_late(void)
{
    unsigned int free_peers = 0U;

    // Figure out how much times we can call
    // enet_host_connect and reallistically succeed
    for(unsigned int i = 0U; i < bother_host->peerCount; ++i) {
        if(bother_host->peers[i].state != ENET_PEER_STATE_DISCONNECTED) {
            continue;
        }

        free_peers += 1U;
    }

    for(unsigned int i = 0U; (i < free_peers) && bother_queue.size(); ++i) {
        const auto& item = bother_queue.front();

        ENetAddress address;
        enet_address_set_host(&address, item.hostname.c_str());
        address.port = enet_uint16(item.port);

        if(auto peer = enet_host_connect(bother_host, &address, 1, 0)) {
            peer->data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(item.identity));
            bother_set.insert(item.identity);
            enet_host_flush(bother_host);
        }

        bother_queue.pop_front();
    }

    ENetEvent enet_event;

    if(0 < enet_host_service(bother_host, &enet_event, 0)) {
        if(enet_event.type == ENET_EVENT_TYPE_CONNECT) {
            protocol::StatusRequest packet;
            packet.game_version_major = version::major;
            protocol::send(enet_event.peer, protocol::encode(packet));
            return;
        }

        if(enet_event.type == ENET_EVENT_TYPE_RECEIVE) {
            protocol::decode(bother_dispatcher, enet_event.packet, enet_event.peer);
            enet_packet_destroy(enet_event.packet);
            return;
        }

        if(enet_event.type == ENET_EVENT_TYPE_DISCONNECT) {
            auto identity = static_cast<unsigned int>(reinterpret_cast<std::uintptr_t>(enet_event.peer->data));

            if(bother_set.count(identity)) {
                BotherResponseEvent event;
                event.identity = identity;
                event.is_server_unreachable = true;
                globals::dispatcher.trigger(event);
            }

            bother_set.erase(identity);

            return;
        }
    }
}

void gui::bother::ping(unsigned int identity, std::string_view host, std::uint16_t port)
{
    if(bother_set.count(identity)) {
        // Already in the process
        return;
    }

    for(const auto& item : bother_queue) {
        if(item.identity == identity) {
            // Already in the queue
            return;
        }
    }

    BotherQueueItem item;
    item.identity = identity;
    item.hostname = host;
    item.port = port;

    bother_queue.push_back(item);
}

void gui::bother::cancel(unsigned int identity)
{
    bother_set.erase(identity);

    auto item = bother_queue.cbegin();

    while(item != bother_queue.cend()) {
        if(item->identity == identity) {
            item = bother_queue.erase(item);
            continue;
        }

        item = std::next(item);
    }

    for(unsigned int i = 0U; i < bother_host->peerCount; ++i) {
        if(bother_host->peers[i].data == reinterpret_cast<void*>(static_cast<std::uintptr_t>(identity))) {
            enet_peer_reset(&bother_host->peers[i]);
            break;
        }
    }
}
