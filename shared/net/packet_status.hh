#ifndef A77BB035_0E49_4D48_89BF_2DE0B21B203F
#define A77BB035_0E49_4D48_89BF_2DE0B21B203F

#include "shared/net/packet.hh"

struct StatusRequest final : public BasePacket<packet_type::STATUS_REQUEST> {
    static void serialize(const StatusRequest& packet, WriteBuffer& buffer);
    static void deserialize(StatusRequest& packet, ReadBuffer& buffer);

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
};

struct StatusResponse final : public BasePacket<packet_type::STATUS_RESPONSE> {
    constexpr static std::uint32_t PASSWORD_PROTECTED = 0x00000001;
    constexpr static std::uint32_t WHITELIST_ENABLED = 0x00000002;

    static void serialize(const StatusResponse& packet, WriteBuffer& buffer);
    static void deserialize(StatusResponse& packet, ReadBuffer& buffer);

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
    std::uint32_t tags;
    std::string motd;
    std::uint16_t players;
    std::uint16_t slots;
};

#endif /* A77BB035_0E49_4D48_89BF_2DE0B21B203F */
