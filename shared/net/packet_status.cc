#include "shared/pch.hh"

#include "shared/net/packet_status.hh"

#include "core/buffer.hh"

void StatusRequest_Packet::encode(const StatusRequest_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.version_major);
    buffer.write<std::uint32_t>(packet.version_minor);
    buffer.write<std::uint32_t>(packet.version_patch);
}

void StatusRequest_Packet::decode(StatusRequest_Packet& packet, ReadBuffer& buffer)
{
    packet.version_major = buffer.read<std::uint32_t>();
    packet.version_minor = buffer.read<std::uint32_t>();
    packet.version_patch = buffer.read<std::uint32_t>();
}

void StatusResponse_Packet::encode(const StatusResponse_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.version_major);
    buffer.write<std::uint16_t>(packet.max_players);
    buffer.write<std::uint16_t>(packet.num_players);
    buffer.write<std::string_view>(packet.motd.substr(0, 32));
    buffer.write<std::uint32_t>(packet.version_minor);
    buffer.write<std::uint32_t>(packet.version_patch);
    buffer.write<std::uint32_t>(packet.server_tags);
}

void StatusResponse_Packet::decode(StatusResponse_Packet& packet, ReadBuffer& buffer)
{
    packet.version_major = buffer.read<std::uint32_t>();
    packet.max_players = buffer.read<std::uint16_t>();
    packet.num_players = buffer.read<std::uint16_t>();
    packet.motd = buffer.read<std::string>().substr(0, 32);
    packet.version_minor = buffer.read<std::uint32_t>();
    packet.version_patch = buffer.read<std::uint32_t>();
    packet.server_tags = buffer.read<std::uint32_t>();
}
