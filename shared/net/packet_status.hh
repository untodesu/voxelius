#ifndef A77BB035_0E49_4D48_89BF_2DE0B21B203F
#define A77BB035_0E49_4D48_89BF_2DE0B21B203F

#include "shared/net/packet.hh"

struct StatusRequest_Packet final : public BasePacket<packet_type::STATUS_REQUEST> {
    static void encode(const StatusRequest_Packet& packet, WriteBuffer& buffer);
    static void decode(StatusRequest_Packet& packet, ReadBuffer& buffer);

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

struct StatusResponse_Packet final : public BasePacket<packet_type::STATUS_RESPONSE> {
    constexpr static std::uint32_t WHITELIST_ENABLED = 0x00000001;
    constexpr static std::uint32_t STRICT_VERSION = 0x00000002;

    static void encode(const StatusResponse_Packet& packet, WriteBuffer& buffer);
    static void decode(StatusResponse_Packet& packet, ReadBuffer& buffer);

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
    std::uint32_t tags;
    std::string motd;
    std::uint16_t players;
    std::uint16_t slots;
};

#endif /* A77BB035_0E49_4D48_89BF_2DE0B21B203F */
