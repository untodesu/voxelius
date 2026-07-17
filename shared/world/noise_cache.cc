#include "shared/pch.hh"

#include "shared/world/noise_cache.hh"

struct NoiseCache_2D::Entry {
    std::once_flag init_flag;
    std::vector<float> values;
};

struct NoiseCache_3D::Entry {
    std::once_flag init_flag;
    std::vector<float> values;
};

NoiseCache_2D::NoiseCache_2D(fnl_state noise, Eigen::Vector2i cell_size)
{
}

float NoiseCache_2D::sample(const BlockPosXZ& pos)
{
}

void NoiseCache_2D::erase(const ChunkPosXZ& pos)
{
}

void NoiseCache_2D::purge(void)
{
}

std::shared_ptr<NoiseCache_2D::Entry> NoiseCache_2D::get_or_create(const ChunkPosXZ& pos)
{
}

void NoiseCache_2D::generate(const ChunkPosXZ& pos, Entry& entry)
{
}

NoiseCache_3D::NoiseCache_3D(fnl_state noise, Eigen::Vector3i cell_size)
{
}

float NoiseCache_3D::sample(const BlockPos& pos)
{
}

void NoiseCache_3D::erase(const ChunkPos& pos)
{
}

void NoiseCache_3D::purge(void)
{
}

std::size_t NoiseCache_3D::size(void) const
{
}

std::shared_ptr<NoiseCache_3D::Entry> NoiseCache_3D::get_or_create(const ChunkPos& pos)
{
}

void NoiseCache_3D::generate(const ChunkPos& pos, Entry& entry)
{
}
