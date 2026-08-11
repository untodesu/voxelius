#ifndef CBB4603F_23BA_4FB8_B70E_E68916DC3A44
#define CBB4603F_23BA_4FB8_B70E_E68916DC3A44

#include "shared/constant.hh"
#include "shared/world/biome.hh"

class ReadBuffer;
class WriteBuffer;

class BiomeSlice final : public std::array<biome_id_type, constant::CHUNK_AREA> {
public:
    static void encode(const BiomeSlice& slice, WriteBuffer& buffer);
    static void decode(BiomeSlice& slice, ReadBuffer& buffer);

    using std::array<biome_id_type, constant::CHUNK_AREA>::array;
};

struct BiomeSlice_Component final {
    std::shared_ptr<BiomeSlice> ptr;
};

#endif /* CBB4603F_23BA_4FB8_B70E_E68916DC3A44 */
