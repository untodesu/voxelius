#include "shared/pch.hh"

#include "shared/net/packet_session.hh"

#include "core/buffer.hh"

void Disconnect::serialize(const Disconnect& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(packet.reason);
}

void Disconnect::deserialize(Disconnect& packet, ReadBuffer& buffer)
{
    packet.reason = buffer.read<std::uint32_t>();
}
