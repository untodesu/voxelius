// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: head.hh
// Description: Head component

#ifndef SHARED_ENTITY_HEAD_HH
#define SHARED_ENTITY_HEAD_HH
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

#endif
