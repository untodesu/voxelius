#ifndef BD2D77E8_727E_4B4C_83D6_92E1569F5B4D
#define BD2D77E8_727E_4B4C_83D6_92E1569F5B4D

#include "shared/net/packet.hh"
#include "shared/world/biome_slice.hh"
#include "shared/world/block_storage.hh"

struct RequestChunk final : public BasePacket<packet_type::REQUEST_CHUNK> {
    static void serialize(const RequestChunk& packet, WriteBuffer& buffer);
    static void deserialize(RequestChunk& packet, ReadBuffer& buffer);

    ChunkPos cpos;
};

struct ChunkBlocks final : public BasePacket<packet_type::CHUNK_BLOCKS> {
    static void serialize(const ChunkBlocks& packet, WriteBuffer& buffer);
    static void deserialize(ChunkBlocks& packet, ReadBuffer& buffer);

    ChunkPos cpos;
    BlockStorage blocks;
};

struct ChunkBiomes final : public BasePacket<packet_type::CHUNK_BIOMES> {
    static void serialize(const ChunkBiomes& packet, WriteBuffer& buffer);
    static void deserialize(ChunkBiomes& packet, ReadBuffer& buffer);

    biome_realm realm;
    ChunkPosXZ cpos;
    BiomeSlice biomes;
};

struct SetBlock final : public BasePacket<packet_type::SET_BLOCK> {
    static void serialize(const SetBlock& packet, WriteBuffer& buffer);
    static void deserialize(SetBlock& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type block;
};

#endif /* BD2D77E8_727E_4B4C_83D6_92E1569F5B4D */
