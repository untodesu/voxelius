#ifndef F73C497B_F125_46D5_AAB3_7F8EC86D13FC
#define F73C497B_F125_46D5_AAB3_7F8EC86D13FC

#include "shared/world/biome.hh"

struct ClimateSample final {
    float temperature;
    float humidity;
    float continentalness;
    float erosion;
    float weirdness;
};

namespace climate
{
void rebuild(void);
} // namespace climate

namespace climate
{
const BiomeDefinition* find(biome_realm realm, const ClimateSample& sample);
float peaks_valleys(float weirdness);
float normalize_01(float sample);
} // namespace climate

#endif /* F73C497B_F125_46D5_AAB3_7F8EC86D13FC */
