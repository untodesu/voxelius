#include "shared/pch.hh"

#include "shared/world/biome_slice.hh"

#include "core/buffer.hh"

void BiomeSlice::encode(const BiomeSlice& slice, WriteBuffer& buffer)
{
    for(std::size_t i = 0; i < constant::CHUNK_AREA; ++i) {
        buffer.write<std::uint32_t>(slice[i]);
    }
}

void BiomeSlice::decode(BiomeSlice& slice, ReadBuffer& buffer)
{
    for(std::size_t i = 0; i < constant::CHUNK_AREA; ++i) {
        slice[i] = buffer.read<std::uint32_t>();
    }
}
