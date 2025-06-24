#ifndef SHARED_TRANSFORM_HH
#define SHARED_TRANSFORM_HH 1
#pragma once

#include "shared/types.hh"

class Dimension;

struct TransformComponent {
    chunk_pos chunk;
    glm::fvec3 local;
    glm::fvec3 angles;

public:
    // Updates TransformComponent values so that
    // the local translation field is always within
    // local coodrinates; [floating-point precision]
    static void fixed_update(Dimension* dimension);
};

// Client-side only - interpolated and previous transform
struct TransformComponentIntr final : public TransformComponent {};
struct TransformComponentPrev final : public TransformComponent {};

#endif /* SHARED_TRANSFORM_HH */
