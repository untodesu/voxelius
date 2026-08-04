#ifndef DDFD985C_53DD_4D07_BCE3_B9644A7AE004
#define DDFD985C_53DD_4D07_BCE3_B9644A7AE004

#include "shared/net/packet.hh"

struct AuthRequest final : public BasePacket<packet_type::AUTH_REQUEST> {
    static void serialize(const AuthRequest& packet, WriteBuffer& buffer);
    static void deserialize(AuthRequest& packet, ReadBuffer& buffer);

    std::uint32_t major;
    std::uint32_t minor;
    std::uint32_t patch;
    std::array<std::uint8_t, 32> pkey;
    std::uint64_t invite;
    std::uint64_t biomes_hash;
    std::uint64_t blocks_hash;
    std::uint64_t fluids_hash;
    std::uint64_t tints_hash;
    std::string username;
};

struct AuthChallenge final : public BasePacket<packet_type::AUTH_CHALLENGE> {
    static void serialize(const AuthChallenge& packet, WriteBuffer& buffer);
    static void deserialize(AuthChallenge& packet, ReadBuffer& buffer);

    std::array<std::uint8_t, 64> nonce;
};

struct AuthResponse final : public BasePacket<packet_type::AUTH_RESPONSE> {
    static void serialize(const AuthResponse& packet, WriteBuffer& buffer);
    static void deserialize(AuthResponse& packet, ReadBuffer& buffer);

    std::array<std::uint8_t, 64> signature;
};

struct AuthAdmission final : public BasePacket<packet_type::AUTH_ADMISSION> {
    static void serialize(const AuthAdmission& packet, WriteBuffer& buffer);
    static void deserialize(AuthAdmission& packet, ReadBuffer& buffer);

    std::string username;
};

#endif /* DDFD985C_53DD_4D07_BCE3_B9644A7AE004 */
