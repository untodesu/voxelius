#ifndef BD2D77E8_727E_4B4C_83D6_92E1569F5B4D
#define BD2D77E8_727E_4B4C_83D6_92E1569F5B4D

#include "shared/net/packet.hh"
#include "shared/world/biome_slice.hh"
#include "shared/world/block_storage.hh"

struct RequestChunk_Packet final : public BasePacket<packet_type::REQUEST_CHUNK> {
    static void serialize(const RequestChunk_Packet& packet, WriteBuffer& buffer);
    static void deserialize(RequestChunk_Packet& packet, ReadBuffer& buffer);

    ChunkPos cpos;
};

struct ChunkBlocks_Packet final : public BasePacket<packet_type::CHUNK_BLOCKS> {
    static void serialize(const ChunkBlocks_Packet& packet, WriteBuffer& buffer);
    static void deserialize(ChunkBlocks_Packet& packet, ReadBuffer& buffer);

    ChunkPos cpos;
    BlockStorage blocks;
};

struct ChunkBiomes_Packet final : public BasePacket<packet_type::CHUNK_BIOMES> {
    static void serialize(const ChunkBiomes_Packet& packet, WriteBuffer& buffer);
    static void deserialize(ChunkBiomes_Packet& packet, ReadBuffer& buffer);

    biome_realm realm;
    ChunkPosXZ cpos;
    BiomeSlice biomes;
};

struct SetBlock_Packet final : public BasePacket<packet_type::SET_BLOCK> {
    static void serialize(const SetBlock_Packet& packet, WriteBuffer& buffer);
    static void deserialize(SetBlock_Packet& packet, ReadBuffer& buffer);

    BlockPos bpos;
    block_id_type block;
};

#endif /* BD2D77E8_727E_4B4C_83D6_92E1569F5B4D */
