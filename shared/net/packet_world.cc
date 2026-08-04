#include "shared/pch.hh"

#include "shared/net/packet_world.hh"

#include "core/buffer.hh"

void RequestChunk_Packet::serialize(const RequestChunk_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int32_t, 3>(packet.cpos.cast<std::int32_t>());
}

void RequestChunk_Packet::deserialize(RequestChunk_Packet& packet, ReadBuffer& buffer)
{
    packet.cpos = buffer.read_vector<std::int32_t, 3>().cast<ChunkPos::value_type>();
}

void ChunkBlocks_Packet::serialize(const ChunkBlocks_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int32_t, 3>(packet.cpos.cast<std::int32_t>());
    BlockStorage::serialize(packet.blocks, buffer);
}

void ChunkBlocks_Packet::deserialize(ChunkBlocks_Packet& packet, ReadBuffer& buffer)
{
    packet.cpos = buffer.read_vector<std::int32_t, 3>().cast<ChunkPos::value_type>();
    BlockStorage::deserialize(packet.blocks, buffer);
}

void ChunkBiomes_Packet::serialize(const ChunkBiomes_Packet& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(packet.realm));
    buffer.write_vector<std::int32_t, 2>(packet.cpos.cast<std::int32_t>());
    BiomeSlice::serialize(packet.biomes, buffer);
}

void ChunkBiomes_Packet::deserialize(ChunkBiomes_Packet& packet, ReadBuffer& buffer)
{
    packet.realm = static_cast<biome_realm>(buffer.read<std::uint32_t>());
    packet.cpos = buffer.read_vector<std::int32_t, 2>().cast<ChunkPosXZ::value_type>();
    BiomeSlice::deserialize(packet.biomes, buffer);
}

void SetBlock_Packet::serialize(const SetBlock_Packet& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int64_t, 3>(packet.bpos.cast<std::int64_t>());
    buffer.write<std::uint32_t>(packet.block);
}

void SetBlock_Packet::deserialize(SetBlock_Packet& packet, ReadBuffer& buffer)
{
    packet.bpos = buffer.read_vector<std::int64_t, 3>().cast<BlockPos::value_type>();
    packet.block = buffer.read<std::uint32_t>();
}
