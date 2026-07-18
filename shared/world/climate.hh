#ifndef C8E4A1B0_CLIMATE_HH
#define C8E4A1B0_CLIMATE_HH

#include "shared/world/biome.hh"

struct ClimateSample final {
    float temperature;
    float humidity;
    float continentalness;
    float weirdness;
};

namespace climate
{
void rebuild(void);
} // namespace climate

namespace climate
{
const BiomeDefinition* find(biome_realm realm, const ClimateSample& sample);
} // namespace climate

#endif /* C8E4A1B0_CLIMATE_HH */
