#ifndef E2AE6925_CAB9_46B0_9B19_3AB5892FC754
#define E2AE6925_CAB9_46B0_9B19_3AB5892FC754

#include "shared/constant.hh"
#include "shared/coord.hh"
#include "shared/world/biome.hh"

class ReadBuffer;
class WriteBuffer;

class BiomeStorage final {
public:
    using array_type = std::array<biome_id_type, constant::CHUNK_AREA>;

    static void serialize(const BiomeStorage* biomes, WriteBuffer& buffer);
    static void deserialize(BiomeStorage* biomes, ReadBuffer& buffer);

    biome_id_type get_biome(std::size_t index) const;
    biome_id_type get_biome(const LocalPosXZ& pos) const;

    void set_biome(std::size_t index, biome_id_type id);
    void set_biome(const LocalPosXZ& pos, biome_id_type id);

    constexpr const array_type& biomes(void) const;
    constexpr std::once_flag& initialized(void);
    void set_biomes(array_type biomes);

private:
    array_type m_biomes;
    std::once_flag m_initialized;
};

constexpr const BiomeStorage::array_type& BiomeStorage::biomes(void) const
{
    return m_biomes;
}

constexpr std::once_flag& BiomeStorage::initialized(void)
{
    return m_initialized;
}

#endif /* E2AE6925_CAB9_46B0_9B19_3AB5892FC754 */
