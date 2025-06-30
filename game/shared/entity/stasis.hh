#ifndef SHARED_ENTITY_STASIS_HH
#define SHARED_ENTITY_STASIS_HH 1
#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity
{
// Attached to entities with transform values
// out of bounds in a specific dimension
struct Stasis final {
public:
    static void fixed_update(world::Dimension* dimension);
};
} // namespace entity

#endif // SHARED_ENTITY_STASIS_HH
