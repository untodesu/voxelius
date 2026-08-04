#include "shared/pch.hh"

#include "shared/net/packet_auth.hh"

#include "core/buffer.hh"

void AuthRequest_Packet::serialize(const AuthRequest_Packet& packet, WriteBuffer& buffer)
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

void AuthRequest_Packet::deserialize(AuthRequest_Packet& packet, ReadBuffer& buffer)
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

void AuthChallenge_Packet::serialize(const AuthChallenge_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_bytes(packet.nonce);
}

void AuthChallenge_Packet::deserialize(AuthChallenge_Packet& packet, ReadBuffer& buffer)
{
    buffer.read_bytes(packet.nonce);
}

void AuthResponse_Packet::serialize(const AuthResponse_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_bytes(packet.signature);
}

void AuthResponse_Packet::deserialize(AuthResponse_Packet& packet, ReadBuffer& buffer)
{
    buffer.read_bytes(packet.signature);
}

void AuthAdmission_Packet::serialize(const AuthAdmission_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint16_t>(packet.client_id);
    buffer.write<std::uint64_t>(packet.identity);
    buffer.write<std::string_view>(packet.username.substr(0, 64));
}

void AuthAdmission_Packet::deserialize(AuthAdmission_Packet& packet, ReadBuffer& buffer)
{
    packet.client_id = buffer.read<std::uint16_t>();
    packet.identity = buffer.read<std::uint64_t>();
    packet.username = buffer.read<std::string>();
}
