#include "shared/pch.hh"

#include "shared/net/packet_player.hh"

#include "core/buffer.hh"

void PlayerAttackE_Packet::serialize(const PlayerAttackE_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.target));
}

void PlayerAttackE_Packet::deserialize(PlayerAttackE_Packet& packet, ReadBuffer& buffer)
{
    packet.target = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}

void PlayerAttackB_Packet::serialize(const PlayerAttackB_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int64_t, 3>(packet.bpos.cast<std::int64_t>());
    buffer.write<std::uint32_t>(packet.expected);
}

void PlayerAttackB_Packet::deserialize(PlayerAttackB_Packet& packet, ReadBuffer& buffer)
{
    packet.bpos = buffer.read_vector<std::int64_t, 3>().cast<BlockPos::value_type>();
    packet.expected = buffer.read<std::uint32_t>();
}

void PlayerInteractE_Packet::serialize(const PlayerInteractE_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.target));
}

void PlayerInteractE_Packet::deserialize(PlayerInteractE_Packet& packet, ReadBuffer& buffer)
{
    packet.target = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}

void PlayerInteractB_Packet::serialize(const PlayerInteractB_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int64_t, 3>(packet.bpos.cast<std::int64_t>());
    buffer.write<std::uint32_t>(packet.expected);
    buffer.write<std::uint8_t>(static_cast<std::uint8_t>(packet.face));
    buffer.write_vector<float, 3>(packet.normal);
    buffer.write_vector<float, 3>(packet.point);
}

void PlayerInteractB_Packet::deserialize(PlayerInteractB_Packet& packet, ReadBuffer& buffer)
{
    packet.bpos = buffer.read_vector<std::int64_t, 3>().cast<BlockPos::value_type>();
    packet.expected = buffer.read<std::uint32_t>();
    packet.face = static_cast<block_face>(buffer.read<std::uint8_t>());
    packet.normal = buffer.read_vector<float, 3>();
    packet.point = buffer.read_vector<float, 3>();
}
