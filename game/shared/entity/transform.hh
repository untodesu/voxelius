#ifndef SHARED_ENTITY_TRANSFORM_HH
#define SHARED_ENTITY_TRANSFORM_HH 1
#pragma once

#include "shared/types.hh"

namespace world
{
class Dimension;
} // namespace world

namespace entity
{
struct Transform {
    chunk_pos chunk;
    glm::fvec3 local;
    glm::fvec3 angles;

public:
    // Updates entity::Transform values so that
    // the local translation field is always within
    // local coodrinates; [floating-point precision]
    static void fixed_update(world::Dimension* dimension);
};
} // namespace entity

namespace entity::client
{
// Client-side only - interpolated and previous transform
struct TransformIntr final : public Transform {};
struct TransformPrev final : public Transform {};
} // namespace entity::client

#endif // SHARED_ENTITY_TRANSFORM_HH
