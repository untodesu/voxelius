#pragma once

#include "shared/const.hh"
#include "shared/types.hh"

namespace io
{
class ReadBuffer;
class WriteBuffer;
} // namespace io

namespace world
{
class VoxelStorage final : public std::array<voxel_id, CHUNK_VOLUME> {
public:
    using std::array<voxel_id, CHUNK_VOLUME>::array;
    void serialize(io::WriteBuffer& buffer) const;
    void deserialize(io::ReadBuffer& buffer);
};
} // namespace world
