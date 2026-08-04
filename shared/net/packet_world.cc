#include "shared/pch.hh"

#include "shared/net/packet_world.hh"

#include "core/buffer.hh"

void RequestChunk::serialize(const RequestChunk& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int32_t, 3>(packet.cpos.cast<std::int32_t>());
}

void RequestChunk::deserialize(RequestChunk& packet, ReadBuffer& buffer)
{
    packet.cpos = buffer.read_vector<std::int32_t, 3>().cast<ChunkPos::value_type>();
}

void ChunkBlocks::serialize(const ChunkBlocks& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int32_t, 3>(packet.cpos.cast<std::int32_t>());
    BlockStorage::serialize(packet.blocks, buffer);
}

void ChunkBlocks::deserialize(ChunkBlocks& packet, ReadBuffer& buffer)
{
    packet.cpos = buffer.read_vector<std::int32_t, 3>().cast<ChunkPos::value_type>();
    BlockStorage::deserialize(packet.blocks, buffer);
}

void ChunkBiomes::serialize(const ChunkBiomes& packet, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(packet.realm));
    buffer.write_vector<std::int32_t, 2>(packet.cpos.cast<std::int32_t>());
    BiomeSlice::serialize(packet.biomes, buffer);
}

void ChunkBiomes::deserialize(ChunkBiomes& packet, ReadBuffer& buffer)
{
    packet.realm = static_cast<biome_realm>(buffer.read<std::uint32_t>());
    packet.cpos = buffer.read_vector<std::int32_t, 2>().cast<ChunkPosXZ::value_type>();
    BiomeSlice::deserialize(packet.biomes, buffer);
}

void SetBlock::serialize(const SetBlock& packet, WriteBuffer& buffer)
{
    buffer.write_vector<std::int64_t, 3>(packet.bpos.cast<std::int64_t>());
    buffer.write<std::uint32_t>(packet.block);
}

void SetBlock::deserialize(SetBlock& packet, ReadBuffer& buffer)
{
    packet.bpos = buffer.read_vector<std::int64_t, 3>().cast<BlockPos::value_type>();
    packet.block = buffer.read<std::uint32_t>();
}
