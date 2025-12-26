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
