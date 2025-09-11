#include "shared/pch.hh"

#include "shared/world/voxel.hh"

#include "core/math/crc64.hh"

#include "shared/world/dimension.hh"

void world::Voxel::on_place(Dimension* dimension, const voxel_pos& vpos) const
{
    // empty
}

void world::Voxel::on_remove(Dimension* dimension, const voxel_pos& vpos) const
{
    // empty
}

void world::Voxel::on_tick(Dimension* dimension, const voxel_pos& vpos) const
{
    // empty
}

void world::Voxel::set_id(voxel_id id) noexcept
{
    m_id = id;
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

std::shared_ptr<world::Voxel> world::Voxel::clone(void) const
{
    return std::make_shared<Voxel>(*this);
}

void world::Voxel::set_name(std::string_view name) noexcept
{
    assert(name.size());

    m_name = name;
}

void world::Voxel::set_render_mode(VoxelRender mode) noexcept
{
    m_render_mode = mode;
}

void world::Voxel::set_shape(VoxelShape shape) noexcept
{
    m_shape = shape;
}

void world::Voxel::set_animated(bool animated) noexcept
{
    m_animated = animated;
}

void world::Voxel::set_touch_type(VoxelTouch type) noexcept
{
    m_touch_type = type;
}

void world::Voxel::set_touch_values(const glm::fvec3& values) noexcept
{
    m_touch_values = values;
}

void world::Voxel::set_surface_material(VoxelMaterial material) noexcept
{
    m_surface_material = material;
}

void world::Voxel::set_collision(const math::AABBf& box) noexcept
{
    m_collision = box;
}

void world::Voxel::add_default_texture(std::string_view path)
{
    assert(path.size());

    m_default_textures.emplace_back(path);
}

void world::Voxel::add_face_texture(VoxelFace face, std::string_view path)
{
    assert(face < m_face_textures.size());
    assert(path.size());

    m_face_textures[face].emplace_back(path);
}
