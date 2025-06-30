#ifndef SHARED_ENTITY_HEAD_HH
#define SHARED_ENTITY_HEAD_HH 1
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

#endif // SHARED_ENTITY_HEAD_HH
