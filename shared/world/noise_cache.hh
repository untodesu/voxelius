#ifndef D8527BD2_E3B7_45CD_A01B_83CA432581A1
#define D8527BD2_E3B7_45CD_A01B_83CA432581A1

#include "shared/constant.hh"
#include "shared/coord.hh"

class NoiseCache_2D final {
public:
    explicit NoiseCache_2D(fnl_state noise, Eigen::Vector2i cell_size);

    NoiseCache_2D(const NoiseCache_2D& other) = delete;
    NoiseCache_2D& operator=(const NoiseCache_2D& other) = delete;

    NoiseCache_2D(NoiseCache_2D&& other) = delete;
    NoiseCache_2D& operator=(NoiseCache_2D&& other) = delete;

    float sample(const BlockPosXZ& pos);

    void erase(const ChunkPosXZ& pos);
    void purge(void);

    std::size_t size(void) const;

private:
    struct Entry;

    std::shared_ptr<Entry> get_or_create(const ChunkPosXZ& pos);

    void generate(const ChunkPosXZ& pos, Entry& entry);

    fnl_state m_noise;
    Eigen::Vector2i m_cell_size;
    Eigen::Vector2i m_grid_size;

    mutable std::mutex m_mutex;
    emhash8::HashMap<ChunkPosXZ, std::shared_ptr<Entry>> m_cache;
};

class NoiseCache_3D final {
public:
    explicit NoiseCache_3D(fnl_state noise, Eigen::Vector3i cell_size);

    NoiseCache_3D(const NoiseCache_3D& other) = delete;
    NoiseCache_3D& operator=(const NoiseCache_3D& other) = delete;

    NoiseCache_3D(NoiseCache_3D&& other) = delete;
    NoiseCache_3D& operator=(NoiseCache_3D&& other) = delete;

    float sample(const BlockPos& pos);

    void erase(const ChunkPos& pos);
    void purge(void);

    std::size_t size(void) const;

private:
    struct Entry;

    std::shared_ptr<Entry> get_or_create(const ChunkPos& pos);

    void generate(const ChunkPos& pos, Entry& entry);

    fnl_state m_noise;
    Eigen::Vector3i m_cell_size;
    Eigen::Vector3i m_grid_size;

    mutable std::mutex m_mutex;
    emhash8::HashMap<ChunkPos, std::shared_ptr<Entry>> m_cache;
};

#endif /* D8527BD2_E3B7_45CD_A01B_83CA432581A1 */
