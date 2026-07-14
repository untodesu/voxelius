#include "shared/pch.hh"

#include "shared/utils/chunk.hh"

#include "shared/constant.hh"

Eigen::AlignedBox3f utils::bounds(const ChunkPos& pos)
{
    auto origin = pos.cast<float>() * static_cast<float>(constant::CHUNK_SIZE);
    auto limit = origin + Eigen::Vector3f::Constant(static_cast<float>(constant::CHUNK_SIZE));
    return Eigen::AlignedBox3f(origin, limit);
}
