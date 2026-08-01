#ifndef C8E4A1F2_9B3D_4E6A_A7C1_5D2F8E0B1947
#define C8E4A1F2_9B3D_4E6A_A7C1_5D2F8E0B1947

#include "shared/world/fluid.hh"

struct AtlasStrip;

struct CachedFluid final {
    const AtlasStrip* albedo_still;
    const AtlasStrip* albedo_flowing;
    const AtlasStrip* mask_still;
    const AtlasStrip* mask_flowing;
};

namespace fluid_cache
{
void init_late(void);
void shutdown(void);
} // namespace fluid_cache

namespace fluid_cache
{
const CachedFluid* find(fluid_id_type id);
} // namespace fluid_cache

#endif /* C8E4A1F2_9B3D_4E6A_A7C1_5D2F8E0B1947 */
