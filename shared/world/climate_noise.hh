#ifndef B3AC6B9D_5DBA_4124_8B5C_6A96B95E227E
#define B3AC6B9D_5DBA_4124_8B5C_6A96B95E227E

#include "shared/coord.hh"
#include "shared/world/climate.hh"

namespace climate_noise
{
void init(std::uint64_t seed);
void shutdown(void);
} // namespace climate_noise

namespace climate_noise
{
ClimateSample sample(const BlockPosXZ& pos);
} // namespace climate_noise

#endif /* B3AC6B9D_5DBA_4124_8B5C_6A96B95E227E */
