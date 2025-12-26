#pragma once

#include "shared/types.hh"

class Dimension;

struct Transform {
    chunk_pos chunk;
    glm::fvec3 local;
    glm::fvec3 angles;

public:
    // Updates Transform values so that
    // the local translation field is always within
    // local coodrinates; [floating-point precision]
    static void fixed_update(Dimension* dimension);
};

namespace client
{
// Client-side only - interpolated and previous transform
struct TransformIntr final : public Transform {};
struct TransformPrev final : public Transform {};
} // namespace client
