#ifndef EE25AA21_7F3F_4B81_B031_17C2207A048A
#define EE25AA21_7F3F_4B81_B031_17C2207A048A

#include "shared/coord.hh"

namespace worldgen
{
void init(void);
void request(const ChunkPos& pos);
bool pending(const ChunkPos& pos);
} // namespace worldgen

#endif /* EE25AA21_7F3F_4B81_B031_17C2207A048A */
