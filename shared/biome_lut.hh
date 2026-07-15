#ifndef D9E3CEFF_AB31_4616_B532_789F22148EDC
#define D9E3CEFF_AB31_4616_B532_789F22148EDC

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

#endif /* D9E3CEFF_AB31_4616_B532_789F22148EDC */
