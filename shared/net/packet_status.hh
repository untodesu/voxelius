#ifndef A77BB035_0E49_4D48_89BF_2DE0B21B203F
#define A77BB035_0E49_4D48_89BF_2DE0B21B203F

#include "shared/net/packet.hh"

struct StatusRequest_Packet final : public BasePacket<packet_type::STATUS_REQUEST> {
    static void encode(const StatusRequest_Packet& packet, WriteBuffer& buffer);
    static void decode(StatusRequest_Packet& packet, ReadBuffer& buffer);

    std::uint32_t version_major;
    std::uint32_t version_minor;
    std::uint32_t version_patch;
};

struct StatusResponse_Packet final : public BasePacket<packet_type::STATUS_RESPONSE> {
    constexpr static std::uint32_t WHITELIST_ENABLED = 0x00000001;
    constexpr static std::uint32_t STRICT_VERSION = 0x00000002;

    static void encode(const StatusResponse_Packet& packet, WriteBuffer& buffer);
    static void decode(StatusResponse_Packet& packet, ReadBuffer& buffer);

    std::uint32_t version_major;
    std::uint16_t max_players;
    std::uint16_t num_players;
    std::string motd;
    std::uint32_t version_minor;
    std::uint32_t version_patch;
    std::uint32_t server_tags;
};

#endif /* A77BB035_0E49_4D48_89BF_2DE0B21B203F */
