#pragma once

struct Head {
    glm::fvec3 angles;
    glm::fvec3 offset;
};

namespace client
{
// Client-side only - interpolated and previous head
struct HeadIntr final : public Head {};
struct HeadPrev final : public Head {};
} // namespace client
