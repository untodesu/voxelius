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
