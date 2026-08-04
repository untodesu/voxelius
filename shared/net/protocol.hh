#ifndef A3E273F9_3BF0_49AA_8E3B_C6E5385F7259
#define A3E273F9_3BF0_49AA_8E3B_C6E5385F7259

#include "shared/net/packet.hh"

namespace protocol
{
constexpr static enet_uint32 CHANNEL = 0;
} // namespace protocol

namespace protocol
{
void decode(const ENetPacket* packet, ENetPeer* peer);
} // namespace protocol

namespace protocol
{
template<typename T>
void broadcast(const T& packet, ENetHost* host, ENetPeer* except = nullptr);
template<typename T>
void send(const T& packet, ENetPeer* peer);
} // namespace protocol

#endif /* A3E273F9_3BF0_49AA_8E3B_C6E5385F7259 */
