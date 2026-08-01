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
    constexpr bool initialized(void) const;

    bool set_biomes(array_type biomes);

private:
    array_type m_biomes {};
    bool m_initialized { false };
};

constexpr const BiomeStorage::array_type& BiomeStorage::biomes(void) const
{
    return m_biomes;
}

constexpr bool BiomeStorage::initialized(void) const
{
    return m_initialized;
}

#endif /* E2AE6925_CAB9_46B0_9B19_3AB5892FC754 */
