#ifndef SHARED_ENTITY_GRAVITY_HH
#define SHARED_ENTITY_GRAVITY_HH 1
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

#endif // SHARED_ENTITY_GRAVITY_HH
