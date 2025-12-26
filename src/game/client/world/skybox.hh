// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: skybox.hh
// Description: Skybox rendering

#ifndef CLIENT_WORLD_SKYBOX_HH
#define CLIENT_WORLD_SKYBOX_HH
#pragma once

namespace skybox
{
extern glm::fvec3 fog_color;
} // namespace skybox

namespace skybox
{
void init(void);
} // namespace skybox

#endif
