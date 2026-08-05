#include "shared/pch.hh"

#include "shared/net/packet_player.hh"

#include "core/buffer.hh"

void PlayerAttackE_Packet::encode(const PlayerAttackE_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.target));
}

void PlayerAttackE_Packet::decode(PlayerAttackE_Packet& packet, ReadBuffer& buffer)
{
    packet.target = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}

void PlayerAttackB_Packet::encode(const PlayerAttackB_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int64_t, 3>(packet.bpos.cast<std::int64_t>());
    buffer.write<std::uint32_t>(packet.expected);
}

void PlayerAttackB_Packet::decode(PlayerAttackB_Packet& packet, ReadBuffer& buffer)
{
    packet.bpos = buffer.read_vector<std::int64_t, 3>().cast<BlockPos::value_type>();
    packet.expected = buffer.read<std::uint32_t>();
}

void PlayerInteractE_Packet::encode(const PlayerInteractE_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint64_t>(static_cast<std::uint64_t>(packet.target));
}

void PlayerInteractE_Packet::decode(PlayerInteractE_Packet& packet, ReadBuffer& buffer)
{
    packet.target = static_cast<entt::entity>(buffer.read<std::uint64_t>());
}

void PlayerInteractB_Packet::encode(const PlayerInteractB_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int64_t, 3>(packet.bpos.cast<std::int64_t>());
    buffer.write<std::uint32_t>(packet.expected);
    buffer.write<std::uint8_t>(static_cast<std::uint8_t>(packet.face));
    buffer.write_vector<float, 3>(packet.normal);
    buffer.write_vector<float, 3>(packet.point);
}

void PlayerInteractB_Packet::decode(PlayerInteractB_Packet& packet, ReadBuffer& buffer)
{
    packet.bpos = buffer.read_vector<std::int64_t, 3>().cast<BlockPos::value_type>();
    packet.expected = buffer.read<std::uint32_t>();
    packet.face = static_cast<block_face>(buffer.read<std::uint8_t>());
    packet.normal = buffer.read_vector<float, 3>();
    packet.point = buffer.read_vector<float, 3>();
}
