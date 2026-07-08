#ifndef AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89
#define AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89

#include "shared/coord.hh"

class Chunk;

namespace world
{
extern emhash8::HashMap<chunk_coord, std::shared_ptr<Chunk>> chunks;
extern entt::regisry chunk_entities;
} // namespace world

#endif /* AC10DC15_9E3A_4C9E_B8B4_E6C5CAE5BB89 */
