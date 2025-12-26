// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: voxel_storage.hh
// Description: Storage for voxels in a chunk

#ifndef SHARED_WORLD_VOXEL_STORAGE_HH
#define SHARED_WORLD_VOXEL_STORAGE_HH
#pragma once

#include "shared/const.hh"
#include "shared/types.hh"

class ReadBuffer;
class WriteBuffer;

class VoxelStorage final : public std::array<voxel_id, CHUNK_VOLUME> {
public:
    using std::array<voxel_id, CHUNK_VOLUME>::array;
    void serialize(WriteBuffer& buffer) const;
    void deserialize(ReadBuffer& buffer);
};

#endif
