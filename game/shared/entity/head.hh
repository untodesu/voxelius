#pragma once

namespace entity
{
struct Head {
    glm::fvec3 angles;
    glm::fvec3 offset;
};
} // namespace entity

namespace entity::client
{
// Client-side only - interpolated and previous head
struct HeadIntr final : public Head {};
struct HeadPrev final : public Head {};
} // namespace entity::client
