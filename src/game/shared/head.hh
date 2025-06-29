#ifndef SHARED_HEAD_HH
#define SHARED_HEAD_HH 1
#pragma once

struct HeadComponent {
    glm::fvec3 angles;
    glm::fvec3 offset;
};

// Client-side only - interpolated and previous head
struct HeadComponentIntr final : public HeadComponent {};
struct HeadComponentPrev final : public HeadComponent {};

#endif // SHARED_HEAD_HH
