// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: voxel_anims.hh
// Description: Voxel texture animations framerate control

#ifndef CLIENT_WORLD_VOXEL_ANIMS_HH
#define CLIENT_WORLD_VOXEL_ANIMS_HH
#pragma once

namespace voxel_anims
{
extern std::uint64_t nextframe;
extern std::uint32_t frame;
} // namespace voxel_anims

namespace voxel_anims
{
void init(void);
void update(void);
} // namespace voxel_anims

#endif
