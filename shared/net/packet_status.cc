#include "shared/pch.hh"

#include "shared/net/packet_status.hh"

#include "core/buffer.hh"

void StatusRequest_Packet::encode(const StatusRequest_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.major);
    buffer.write<std::uint32_t>(packet.minor);
    buffer.write<std::uint32_t>(packet.patch);
}

void StatusRequest_Packet::decode(StatusRequest_Packet& packet, ReadBuffer& buffer)
{
    packet.major = buffer.read<std::uint32_t>();
    packet.minor = buffer.read<std::uint32_t>();
    packet.patch = buffer.read<std::uint32_t>();
}

void StatusResponse_Packet::encode(const StatusResponse_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.major);
    buffer.write<std::uint32_t>(packet.minor);
    buffer.write<std::uint32_t>(packet.patch);
    buffer.write<std::uint32_t>(packet.tags);
    buffer.write<std::string_view>(packet.motd.substr(0, 32));
    buffer.write<std::uint16_t>(packet.players);
    buffer.write<std::uint16_t>(packet.slots);
}

void StatusResponse_Packet::decode(StatusResponse_Packet& packet, ReadBuffer& buffer)
{
    packet.major = buffer.read<std::uint32_t>();
    packet.minor = buffer.read<std::uint32_t>();
    packet.patch = buffer.read<std::uint32_t>();
    packet.tags = buffer.read<std::uint32_t>();
    packet.motd = buffer.read<std::string>();
    packet.players = buffer.read<std::uint16_t>();
    packet.slots = buffer.read<std::uint16_t>();
}
