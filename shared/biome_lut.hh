#ifndef C7B4E2A1_3F6D_4A8E_9C0B_1D2E3F4A5B6C
#define C7B4E2A1_3F6D_4A8E_9C0B_1D2E3F4A5B6C

#include "shared/biome.hh"

namespace biome_lut
{
void rebuild(void);
void purge(void);
} // namespace biome_lut

namespace biome_lut
{
biome_id_type lookup(biome_realm realm, int temperature, int humidity, int axis);
} // namespace biome_lut

#endif /* C7B4E2A1_3F6D_4A8E_9C0B_1D2E3F4A5B6C */
