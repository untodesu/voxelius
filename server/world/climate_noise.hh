#ifndef B3AC6B9D_5DBA_4124_8B5C_6A96B95E227E
#define B3AC6B9D_5DBA_4124_8B5C_6A96B95E227E

#include "shared/coord.hh"

#include "server/constant.hh"
#include "server/world/climate.hh"

using ClimateSampleArray = std::array<ClimateSample, constant::CHUNK_AREA>;

namespace climate_noise
{
void init(std::mt19937_64& seeder);
void shutdown(void);
} // namespace climate_noise

namespace climate_noise
{
ClimateSample sample_block(const BlockPosXZ& pos);
ClimateSampleArray sample_array(const ChunkPosXZ& pos);
} // namespace climate_noise

#endif /* B3AC6B9D_5DBA_4124_8B5C_6A96B95E227E */
