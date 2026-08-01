#include "shared/pch.hh"

#include "shared/world/biome_storage.hh"

#include "core/buffer.hh"

void BiomeStorage::serialize(const BiomeStorage* biomes, WriteBuffer& buffer)
{
    for(std::size_t i = 0; i < constant::CHUNK_AREA; ++i) {
        buffer.write<std::uint32_t>(biomes->m_biomes[i]);
    }
}

void BiomeStorage::deserialize(BiomeStorage* biomes, ReadBuffer& buffer)
{
    for(std::size_t i = 0; i < constant::CHUNK_AREA; ++i) {
        biomes->m_biomes[i] = buffer.read<std::uint32_t>();
    }

    biomes->m_initialized = true;
}

biome_id_type BiomeStorage::get_biome(std::size_t index) const
{
    return m_biomes[index];
}

biome_id_type BiomeStorage::get_biome(const LocalPosXZ& pos) const
{
    auto size_x = static_cast<std::size_t>(pos[0]);
    auto size_z = static_cast<std::size_t>(pos[1]);
    auto index = size_x + (size_z * constant::CHUNK_SIZE);
    return m_biomes[index];
}

void BiomeStorage::set_biome(std::size_t index, biome_id_type biome)
{
    m_biomes[index] = biome;
}

void BiomeStorage::set_biome(const LocalPosXZ& pos, biome_id_type biome)
{
    auto size_x = static_cast<std::size_t>(pos[0]);
    auto size_z = static_cast<std::size_t>(pos[1]);
    auto index = size_x + (size_z * constant::CHUNK_SIZE);
    m_biomes[index] = biome;
}

bool BiomeStorage::set_biomes(array_type biomes)
{
    // First write wins — refuse if the column was already initialized.
    if(m_initialized) {
        return false;
    }

    m_biomes = std::move(biomes);
    m_initialized = true;
    return true;
}
