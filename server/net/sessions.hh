#ifndef CB1D9924_5E4E_4165_AAD7_7B58D385FFA7
#define CB1D9924_5E4E_4165_AAD7_7B58D385FFA7

#include "shared/net/ed25519.hh"

enum class session_state : std::uint8_t {
    UNCONNECTED,
    CHALLENGE,
    CONNECTED,
};

struct Session final {
    session_state state;

    ed25519::pkey_type pkey;
    std::array<std::byte, 64> nonce;
    std::vector<std::byte> salted_nonce;

    std::uint16_t client_id;
    std::uint64_t identity;
    std::string username;
    entt::entity player;

    ENetPeer* peer;
};

struct SessionRef final {
    Session* ptr;
};

namespace sessions
{
extern std::uint16_t num_players;
} // namespace sessions

namespace sessions
{
void init(void);
void init_late(void);
void shutdown(void);
} // namespace sessions

namespace sessions
{
Session* create(ENetPeer* peer, std::string_view username);
Session* find(std::string_view username);
Session* find(std::uint16_t client_id);
Session* find(std::uint64_t identity);
Session* find(const ENetPeer* peer);
void destroy(Session* session, std::optional<std::uint32_t> reason = std::nullopt);
} // namespace sessions

#endif /* CB1D9924_5E4E_4165_AAD7_7B58D385FFA7 */
