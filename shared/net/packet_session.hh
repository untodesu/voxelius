#ifndef BE0B58E2_0871_46F1_830D_2C8D9E4967DD
#define BE0B58E2_0871_46F1_830D_2C8D9E4967DD

#include "shared/net/packet.hh"

struct Disconnect final : public BasePacket<packet_type::DISCONNECT> {
    constexpr static std::uint32_t UNSPECIFIED = 0x00000000;
    constexpr static std::uint32_t CLIENT_DISCONNECT = 0x00000001;
    constexpr static std::uint32_t CLIENT_SHUTDOWN = 0x00000002;
    constexpr static std::uint32_t ENTITY_ID_DESYNC = 0x00000003;
    constexpr static std::uint32_t CHECKSUM_MISMATCH = 0x00000004;
    constexpr static std::uint32_t NOT_WHITELISTED = 0x00000005;
    constexpr static std::uint32_t INVALID_SIGNATURE = 0x00000006;
    constexpr static std::uint32_t OUTDATED_CLIENT = 0x00000007;
    constexpr static std::uint32_t OUTDATED_SERVER = 0x00000008;
    constexpr static std::uint32_t SERVER_IS_FULL = 0x00000009;
    constexpr static std::uint32_t SERVER_SHUTDOWN = 0x0000000A;

    static void serialize(const Disconnect& packet, WriteBuffer& buffer);
    static void deserialize(Disconnect& packet, ReadBuffer& buffer);

    std::uint32_t reason;
};

#endif /* BE0B58E2_0871_46F1_830D_2C8D9E4967DD */
