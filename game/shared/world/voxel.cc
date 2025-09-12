#include "shared/pch.hh"

#include "shared/world/voxel.hh"

#include "core/math/crc64.hh"

#include "shared/world/dimension.hh"

world::Voxel::Voxel(const Voxel& source, voxel_id id) noexcept : Voxel(source)
{
    m_id = id;
}

void world::Voxel::on_place(Dimension* dimension, const voxel_pos& vpos) const
{
    if(m_on_place) {
        m_on_place(dimension, vpos);
    }
}

void world::Voxel::on_remove(Dimension* dimension, const voxel_pos& vpos) const
{
    if(m_on_remove) {
        m_on_remove(dimension, vpos);
    }
}

void world::Voxel::on_tick(Dimension* dimension, const voxel_pos& vpos) const
{
    if(m_on_tick) {
        m_on_tick(dimension, vpos);
    }
}

std::size_t world::Voxel::get_random_texture_index(VoxelFace face, const voxel_pos& vpos) const
{
    const auto& textures = get_face_textures(face);

    assert(textures.size());

    std::uint64_t hash = 0U;
    hash = math::crc64(&vpos.x, sizeof(vpos.x), hash);
    hash = math::crc64(&vpos.y, sizeof(vpos.y), hash);
    hash = math::crc64(&vpos.z, sizeof(vpos.z), hash);

    return static_cast<std::size_t>(hash % textures.size());
}

void world::Voxel::set_face_cache(VoxelFace face, std::size_t offset, std::size_t plane)
{
    assert(face < m_cached_face_offsets.size());
    assert(face < m_cached_face_planes.size());

    m_cached_face_offsets[face] = offset;
    m_cached_face_planes[face] = plane;
}

std::uint64_t world::Voxel::calculate_checksum(std::uint64_t combine) const
{
    combine = math::crc64(m_name.data(), m_name.size(), combine);
    combine += static_cast<std::uint64_t>(m_shape);
    combine += static_cast<std::uint64_t>(m_render_mode);
    return combine;
}

world::VoxelBuilder::VoxelBuilder(std::string_view name)
{
    set_name(name);
}

world::VoxelBuilder& world::VoxelBuilder::set_on_place(VoxelOnPlaceFunc func) noexcept
{
    m_on_place = std::move(func);

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_on_remove(VoxelOnRemoveFunc func) noexcept
{
    m_on_remove = std::move(func);

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_on_tick(VoxelOnTickFunc func) noexcept
{
    m_on_tick = std::move(func);

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_name(std::string_view name) noexcept
{
    assert(name.size());

    m_name = name;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_render_mode(VoxelRender mode) noexcept
{
    m_render_mode = mode;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_shape(VoxelShape shape) noexcept
{
    m_shape = shape;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_animated(bool animated) noexcept
{
    m_animated = animated;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_touch_type(VoxelTouch type) noexcept
{
    m_touch_type = type;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_touch_values(const glm::fvec3& values) noexcept
{
    m_touch_values = values;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_surface_material(VoxelMaterial material) noexcept
{
    m_surface_material = material;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::set_collision(const math::AABBf& box) noexcept
{
    m_collision = box;

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::add_default_texture(std::string_view path)
{
    assert(path.size());

    m_default_textures.emplace_back(path);

    return *this;
}

world::VoxelBuilder& world::VoxelBuilder::add_face_texture(VoxelFace face, std::string_view path)
{
    assert(face < m_face_textures.size());
    assert(path.size());

    m_face_textures[face].emplace_back(path);

    return *this;
}

std::unique_ptr<world::Voxel> world::VoxelBuilder::build(voxel_id id) const
{
    assert(m_name.size());
    assert(id);

    return std::make_unique<Voxel>(*this, id);
}
