#include "shared/pch.hh"

#include "shared/net/packet_auth.hh"

#include "core/buffer.hh"

void AuthRequest::serialize(const AuthRequest& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.major);
    buffer.write<std::uint32_t>(packet.minor);
    buffer.write<std::uint32_t>(packet.patch);
    buffer.write_bytes(packet.pkey);
    buffer.write<std::uint64_t>(packet.invite);
    buffer.write<std::uint64_t>(packet.biomes_hash);
    buffer.write<std::uint64_t>(packet.blocks_hash);
    buffer.write<std::uint64_t>(packet.fluids_hash);
    buffer.write<std::uint64_t>(packet.tints_hash);
    buffer.write<std::string_view>(packet.username.substr(0, 64));
}

void AuthRequest::deserialize(AuthRequest& packet, ReadBuffer& buffer)
{
    packet.major = buffer.read<std::uint32_t>();
    packet.minor = buffer.read<std::uint32_t>();
    packet.patch = buffer.read<std::uint32_t>();
    buffer.read_bytes(packet.pkey);
    packet.invite = buffer.read<std::uint64_t>();
    packet.biomes_hash = buffer.read<std::uint64_t>();
    packet.blocks_hash = buffer.read<std::uint64_t>();
    packet.fluids_hash = buffer.read<std::uint64_t>();
    packet.tints_hash = buffer.read<std::uint64_t>();
    packet.username = buffer.read<std::string>();
}

void AuthChallenge::serialize(const AuthChallenge& packet, WriteBuffer& buffer)
{
    buffer.write_bytes(packet.nonce);
}

void AuthChallenge::deserialize(AuthChallenge& packet, ReadBuffer& buffer)
{
    buffer.read_bytes(packet.nonce);
}

void AuthResponse::serialize(const AuthResponse& packet, WriteBuffer& buffer)
{
    buffer.write_bytes(packet.signature);
}

void AuthResponse::deserialize(AuthResponse& packet, ReadBuffer& buffer)
{
    buffer.read_bytes(packet.signature);
}

void AuthAdmission::serialize(const AuthAdmission& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint16_t>(packet.client_id);
    buffer.write<std::uint64_t>(packet.identity);
    buffer.write<std::string_view>(packet.username.substr(0, 64));
}

void AuthAdmission::deserialize(AuthAdmission& packet, ReadBuffer& buffer)
{
    packet.client_id = buffer.read<std::uint16_t>();
    packet.identity = buffer.read<std::uint64_t>();
    packet.username = buffer.read<std::string>();
}
