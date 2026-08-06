#include "client/pch.hh"

#include "client/net/bother.hh"

#include "core/buffer.hh"
#include "core/version.hh"

#include "shared/net/packet_status.hh"
#include "shared/net/protocol.hh"

#include "client/globals.hh"

constexpr static std::uint32_t MAX_PEERS = 16;
constexpr static std::uint32_t SERVICE_TIMEOUT_MS = 200;

struct Job_Ping final {
    unsigned request_id;
    std::string host;
    std::uint16_t port;
};

struct Job_Cancel final {
    unsigned request_id;
};

using Job = std::variant<Job_Ping, Job_Cancel>;

struct BotherData final {
    unsigned request_id;
    bool responded;
};

static std::jthread s_thread;
static std::mutex s_incoming_mutex;
static std::vector<Job> s_incoming;
static std::mutex s_outgoing_mutex;
static std::vector<BotherResponseEvent> s_outgoing;

BotherResponseEvent::BotherResponseEvent(unsigned request_id) : m_request_id(request_id), m_unreachable(true)
{
    // empty
}

BotherResponseEvent::BotherResponseEvent(unsigned request_id, std::uint16_t num_players, std::uint16_t max_players,
    std::uint32_t version_major, std::uint32_t version_minor, std::uint32_t version_patch, const std::string& motd)
    : m_request_id(request_id), m_unreachable(false)
{
    m_num_players = num_players;
    m_max_players = max_players;
    m_version_major = version_major;
    m_version_minor = version_minor;
    m_version_patch = version_patch;
    m_motd = std::string(motd);
}

static void push_outgoing(BotherResponseEvent event)
{
    std::scoped_lock lock(s_outgoing_mutex);

    s_outgoing.emplace_back(std::move(event));
}

static void on_status_response(const StatusResponse_Packet& packet)
{
    auto data = reinterpret_cast<BotherData*>(packet.peer->data);

    if(data == nullptr) {
        return;
    }

    data->responded = true;

    push_outgoing(BotherResponseEvent(data->request_id, packet.players, packet.slots, packet.major, packet.minor, packet.patch,
        packet.motd));

    enet_peer_disconnect(packet.peer, 0);
}

static std::vector<Job> take_incoming(void)
{
    std::scoped_lock lock(s_incoming_mutex);

    std::vector<Job> jobs;
    jobs.swap(s_incoming);

    return jobs;
}

static std::vector<BotherResponseEvent> take_outgoing(void)
{
    std::scoped_lock lock(s_outgoing_mutex);

    std::vector<BotherResponseEvent> events;
    events.swap(s_outgoing);

    return events;
}

static void thread_main(std::stop_token stop_token)
{
    auto host = enet_host_create(nullptr, MAX_PEERS, 1, 0, 0);

    if(host == nullptr) {
        LOG_WARNING("failed to create host");
        return;
    }

    entt::dispatcher dispatcher;
    dispatcher.sink<StatusResponse_Packet>().connect<&on_status_response>();

    while(!stop_token.stop_requested()) {
        auto jobs = take_incoming();

        for(const auto& job : jobs) {
            if(auto ping = std::get_if<Job_Ping>(&job)) {
                ENetAddress address {};
                address.port = ping->port;

                if(enet_address_set_host(&address, ping->host.c_str()) < 0) {
                    push_outgoing(BotherResponseEvent(ping->request_id));
                    continue;
                }

                auto peer = enet_host_connect(host, &address, 1, 0);

                if(peer == nullptr) {
                    push_outgoing(BotherResponseEvent(ping->request_id));
                    continue;
                }

                auto data = new BotherData;
                data->request_id = ping->request_id;
                data->responded = false;
                peer->data = data;
            }
            else if(auto cancel = std::get_if<Job_Cancel>(&job)) {
                for(size_t i = 0; i < host->peerCount; ++i) {
                    auto peer = &host->peers[i];
                    auto data = reinterpret_cast<BotherData*>(peer->data);

                    if(data && data->request_id == cancel->request_id) {
                        enet_peer_reset(peer);
                        delete data;
                        peer->data = nullptr;
                        break;
                    }
                }
            }
        }

        ENetEvent event {};

        while(0 < enet_host_service(host, &event, SERVICE_TIMEOUT_MS)) {
            if(event.type == ENET_EVENT_TYPE_CONNECT) {
                StatusRequest_Packet request {};
                request.major = version::major;
                request.minor = version::minor;
                request.patch = version::patch;
                protocol::send(request, event.peer);
                continue;
            }

            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                protocol::decode(dispatcher, event.packet, event.peer);
                enet_packet_destroy(event.packet);
                continue;
            }

            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                auto data = reinterpret_cast<BotherData*>(event.peer->data);

                if(data && !data->responded) {
                    push_outgoing(BotherResponseEvent(data->request_id));
                    delete data;
                }

                event.peer->data = nullptr;

                continue;
            }
        }
    }

    for(size_t i = 0; i < host->peerCount; ++i) {
        auto peer = &host->peers[i];
        auto data = reinterpret_cast<BotherData*>(peer->data);

        if(data && !data->responded) {
            push_outgoing(BotherResponseEvent(data->request_id));
            delete data;
        }

        peer->data = nullptr;
    }

    enet_host_destroy(host);
}

void bother::init(void)
{
    s_thread = std::jthread(&thread_main);
}

void bother::shutdown(void)
{
    s_thread.request_stop();

    if(s_thread.joinable()) {
        s_thread.join();
    }
}

void bother::update_late(void)
{
    auto events = take_outgoing();

    for(auto& event : events) {
        globals::dispatcher.trigger(std::move(event));
    }
}

void bother::ping(unsigned int request_id, std::string_view host, std::uint16_t port)
{
    Job_Ping job {};
    job.request_id = request_id;
    job.host = std::string(host);
    job.port = port;

    std::scoped_lock lock(s_incoming_mutex);
    s_incoming.emplace_back(std::move(job));
}

void bother::cancel(unsigned int request_id)
{
    Job_Cancel job {};
    job.request_id = request_id;

    std::scoped_lock lock(s_incoming_mutex);
    s_incoming.emplace_back(std::move(job));
}
