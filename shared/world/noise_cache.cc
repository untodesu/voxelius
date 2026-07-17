#include "shared/pch.hh"

#include "shared/world/noise_cache.hh"

#include "core/exception.hh"

struct NoiseCache_2D::Entry {
    std::once_flag init_flag;
    std::vector<float> values;
};

struct NoiseCache_3D::Entry {
    std::once_flag init_flag;
    std::vector<float> values;
};

static std::size_t get_index_2d(const Eigen::Vector2i& grid_size, const Eigen::Vector2i& cell_pos)
{
    return static_cast<std::size_t>(cell_pos.y() * grid_size.x() + cell_pos.x());
}

static std::size_t get_index_3d(const Eigen::Vector3i& grid_size, const Eigen::Vector3i& cell_pos)
{
    return static_cast<std::size_t>(cell_pos.z() * grid_size.x() * grid_size.y() + cell_pos.y() * grid_size.x() + cell_pos.x());
}

NoiseCache_2D::NoiseCache_2D(fnl_state noise, Eigen::Vector2i cell_size) : m_noise(std::move(noise)), m_cell_size(cell_size)
{
    auto cell_size_valid = true;
    cell_size_valid = cell_size_valid && cell_size.x() > 0;
    cell_size_valid = cell_size_valid && cell_size.y() > 0;
    cell_size_valid = cell_size_valid && constant::CHUNK_SIZE % cell_size.x() == 0;
    cell_size_valid = cell_size_valid && constant::CHUNK_SIZE % cell_size.y() == 0;
    vx::throw_if_not_fmt(cell_size_valid, "invalid cell size: {}x{}", cell_size.x(), cell_size.y());

    m_grid_size.x() = static_cast<int>(constant::CHUNK_SIZE) / cell_size.x() + 1;
    m_grid_size.y() = static_cast<int>(constant::CHUNK_SIZE) / cell_size.y() + 1;
}

float NoiseCache_2D::sample(const BlockPosXZ& pos)
{
    ChunkPosXZ cpos;
    cpos.x() = static_cast<ChunkPosXZ::value_type>(pos.x() >> constant::CHUNK_SIZE_LOG2);
    cpos.y() = static_cast<ChunkPosXZ::value_type>(pos.y() >> constant::CHUNK_SIZE_LOG2);

    BlockPosXZ chunk_origin;
    chunk_origin.x() = static_cast<BlockPosXZ::value_type>(cpos.x()) * constant::CHUNK_SIZE;
    chunk_origin.y() = static_cast<BlockPosXZ::value_type>(cpos.y()) * constant::CHUNK_SIZE;

    BlockPosXZ local_pos = pos - chunk_origin;
    Eigen::Vector2i cell_pos;
    Eigen::Vector2f alpha;

    cell_pos.x() = static_cast<int>(local_pos.x()) / m_cell_size.x();
    cell_pos.y() = static_cast<int>(local_pos.y()) / m_cell_size.y();

    alpha.x() = static_cast<float>(local_pos.x() % m_cell_size.x()) / static_cast<float>(m_cell_size.x());
    alpha.y() = static_cast<float>(local_pos.y() % m_cell_size.y()) / static_cast<float>(m_cell_size.y());

    auto entry = get_or_create(cpos);

    std::call_once(entry->init_flag, [this, cpos, entry] {
        generate(cpos, entry.get());
    });

    auto x0_a = entry->values.at(get_index_2d(m_grid_size, cell_pos));
    auto x0_b = entry->values.at(get_index_2d(m_grid_size, cell_pos + Eigen::Vector2i(1, 0)));
    auto x0_lerp = std::lerp(x0_a, x0_b, alpha.x());

    auto x1_a = entry->values.at(get_index_2d(m_grid_size, cell_pos + Eigen::Vector2i(0, 1)));
    auto x1_b = entry->values.at(get_index_2d(m_grid_size, cell_pos + Eigen::Vector2i(1, 1)));
    auto x1_lerp = std::lerp(x1_a, x1_b, alpha.x());

    return std::lerp(x0_lerp, x1_lerp, alpha.y());
}

void NoiseCache_2D::erase(const ChunkPosXZ& pos)
{
    std::scoped_lock lock(m_mutex);

    m_cache.erase(pos);
}

void NoiseCache_2D::purge(void)
{
    std::scoped_lock lock(m_mutex);

    m_cache.clear();
}

std::size_t NoiseCache_2D::size(void) const
{
    std::scoped_lock lock(m_mutex);

    return m_cache.size();
}

std::shared_ptr<NoiseCache_2D::Entry> NoiseCache_2D::get_or_create(const ChunkPosXZ& pos)
{
    std::scoped_lock lock(m_mutex);

    auto it = m_cache.find(pos);

    if(it == m_cache.cend()) {
        auto entry = std::make_shared<Entry>();
        m_cache.emplace(pos, entry);
        return entry;
    }

    return it->second;
}

void NoiseCache_2D::generate(const ChunkPosXZ& pos, Entry* entry)
{
    auto size_x = static_cast<std::size_t>(m_grid_size.x());
    auto size_y = static_cast<std::size_t>(m_grid_size.y());
    entry->values.resize(size_x * size_y);

    BlockPosXZ chunk_origin;
    chunk_origin.x() = static_cast<BlockPosXZ::value_type>(pos.x()) * constant::CHUNK_SIZE;
    chunk_origin.y() = static_cast<BlockPosXZ::value_type>(pos.y()) * constant::CHUNK_SIZE;

    fnl_state noise = m_noise; // copy is intentional

    for(int y = 0; y < m_grid_size.y(); ++y) {
        for(int x = 0; x < m_grid_size.x(); ++x) {
            BlockPosXZ sample_pos;
            sample_pos.x() = chunk_origin.x() + static_cast<BlockPosXZ::value_type>(x * m_cell_size.x());
            sample_pos.y() = chunk_origin.y() + static_cast<BlockPosXZ::value_type>(y * m_cell_size.y());

            auto index = get_index_2d(m_grid_size, Eigen::Vector2i(x, y));
            auto sample_x = static_cast<FNLfloat>(sample_pos.x());
            auto sample_y = static_cast<FNLfloat>(sample_pos.y());

            entry->values[index] = fnlGetNoise2D(&noise, sample_x, sample_y);
        }
    }
}

NoiseCache_3D::NoiseCache_3D(fnl_state noise, Eigen::Vector3i cell_size) : m_noise(std::move(noise)), m_cell_size(cell_size)
{
    auto cell_size_valid = true;
    cell_size_valid = cell_size_valid && cell_size.x() > 0;
    cell_size_valid = cell_size_valid && cell_size.y() > 0;
    cell_size_valid = cell_size_valid && cell_size.z() > 0;
    cell_size_valid = cell_size_valid && constant::CHUNK_SIZE % cell_size.x() == 0;
    cell_size_valid = cell_size_valid && constant::CHUNK_SIZE % cell_size.y() == 0;
    cell_size_valid = cell_size_valid && constant::CHUNK_SIZE % cell_size.z() == 0;
    vx::throw_if_not_fmt(cell_size_valid, "invalid cell size: {}x{}x{}", cell_size.x(), cell_size.y(), cell_size.z());

    m_grid_size.x() = static_cast<int>(constant::CHUNK_SIZE) / cell_size.x() + 1;
    m_grid_size.y() = static_cast<int>(constant::CHUNK_SIZE) / cell_size.y() + 1;
    m_grid_size.z() = static_cast<int>(constant::CHUNK_SIZE) / cell_size.z() + 1;
}

float NoiseCache_3D::sample(const BlockPos& pos)
{
    ChunkPos chunk_pos;
    chunk_pos.x() = static_cast<ChunkPos::value_type>(pos.x() >> constant::CHUNK_SIZE_LOG2);
    chunk_pos.y() = static_cast<ChunkPos::value_type>(pos.y() >> constant::CHUNK_SIZE_LOG2);
    chunk_pos.z() = static_cast<ChunkPos::value_type>(pos.z() >> constant::CHUNK_SIZE_LOG2);

    BlockPos chunk_origin;
    chunk_origin.x() = static_cast<BlockPos::value_type>(chunk_pos.x()) * constant::CHUNK_SIZE;
    chunk_origin.y() = static_cast<BlockPos::value_type>(chunk_pos.y()) * constant::CHUNK_SIZE;
    chunk_origin.z() = static_cast<BlockPos::value_type>(chunk_pos.z()) * constant::CHUNK_SIZE;

    BlockPos local_pos = pos - chunk_origin;
    Eigen::Vector3i cell_pos;
    Eigen::Vector3f alpha;

    cell_pos.x() = static_cast<int>(local_pos.x()) / m_cell_size.x();
    cell_pos.y() = static_cast<int>(local_pos.y()) / m_cell_size.y();
    cell_pos.z() = static_cast<int>(local_pos.z()) / m_cell_size.z();

    alpha.x() = static_cast<float>(local_pos.x() % m_cell_size.x()) / static_cast<float>(m_cell_size.x());
    alpha.y() = static_cast<float>(local_pos.y() % m_cell_size.y()) / static_cast<float>(m_cell_size.y());
    alpha.z() = static_cast<float>(local_pos.z() % m_cell_size.z()) / static_cast<float>(m_cell_size.z());

    auto entry = get_or_create(chunk_pos);

    std::call_once(entry->init_flag, [this, chunk_pos, entry] {
        generate(chunk_pos, entry.get());
    });

    auto x00_a = entry->values.at(get_index_3d(m_grid_size, cell_pos));
    auto x00_b = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(1, 0, 0)));
    auto x00_lerp = std::lerp(x00_a, x00_b, alpha.x());

    auto x01_a = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(0, 0, 1)));
    auto x01_b = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(1, 0, 1)));
    auto x01_lerp = std::lerp(x01_a, x01_b, alpha.x());

    auto x10_a = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(0, 1, 0)));
    auto x10_b = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(1, 1, 0)));
    auto x10_lerp = std::lerp(x10_a, x10_b, alpha.x());

    auto x11_a = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(0, 1, 1)));
    auto x11_b = entry->values.at(get_index_3d(m_grid_size, cell_pos + Eigen::Vector3i(1, 1, 1)));
    auto x11_lerp = std::lerp(x11_a, x11_b, alpha.x());

    auto z0_lerp = std::lerp(x00_lerp, x01_lerp, alpha.z());
    auto z1_lerp = std::lerp(x10_lerp, x11_lerp, alpha.z());

    return std::lerp(z0_lerp, z1_lerp, alpha.y());
}

void NoiseCache_3D::erase(const ChunkPos& pos)
{
    std::scoped_lock lock(m_mutex);

    m_cache.erase(pos);
}

void NoiseCache_3D::purge(void)
{
    std::scoped_lock lock(m_mutex);

    m_cache.clear();
}

std::size_t NoiseCache_3D::size(void) const
{
    std::scoped_lock lock(m_mutex);

    return m_cache.size();
}

std::shared_ptr<NoiseCache_3D::Entry> NoiseCache_3D::get_or_create(const ChunkPos& pos)
{
    std::scoped_lock lock(m_mutex);

    auto it = m_cache.find(pos);

    if(it == m_cache.cend()) {
        auto entry = std::make_shared<Entry>();
        m_cache.emplace(pos, entry);
        return entry;
    }

    return it->second;
}

void NoiseCache_3D::generate(const ChunkPos& pos, Entry* entry)
{
    auto size_x = static_cast<std::size_t>(m_grid_size.x());
    auto size_y = static_cast<std::size_t>(m_grid_size.y());
    auto size_z = static_cast<std::size_t>(m_grid_size.z());
    entry->values.resize(size_x * size_y * size_z);

    BlockPos chunk_origin;
    chunk_origin.x() = static_cast<BlockPos::value_type>(pos.x()) * constant::CHUNK_SIZE;
    chunk_origin.y() = static_cast<BlockPos::value_type>(pos.y()) * constant::CHUNK_SIZE;
    chunk_origin.z() = static_cast<BlockPos::value_type>(pos.z()) * constant::CHUNK_SIZE;

    fnl_state noise = m_noise; // copy is intentional

    for(int z = 0; z < m_grid_size.z(); ++z) {
        for(int y = 0; y < m_grid_size.y(); ++y) {
            for(int x = 0; x < m_grid_size.x(); ++x) {
                BlockPos sample_pos;
                sample_pos.x() = chunk_origin.x() + static_cast<BlockPos::value_type>(x * m_cell_size.x());
                sample_pos.y() = chunk_origin.y() + static_cast<BlockPos::value_type>(y * m_cell_size.y());
                sample_pos.z() = chunk_origin.z() + static_cast<BlockPos::value_type>(z * m_cell_size.z());

                auto index = get_index_3d(m_grid_size, Eigen::Vector3i(x, y, z));
                auto sample_x = static_cast<FNLfloat>(sample_pos.x());
                auto sample_y = static_cast<FNLfloat>(sample_pos.y());
                auto sample_z = static_cast<FNLfloat>(sample_pos.z());

                entry->values[index] = fnlGetNoise3D(&noise, sample_x, sample_y, sample_z);
            }
        }
    }
}
