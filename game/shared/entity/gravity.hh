#pragma once

namespace world
{
class Dimension;
} // namespace world

namespace entity
{
struct Gravity final {
public:
    static void fixed_update(world::Dimension* dimension);
};
} // namespace entity
