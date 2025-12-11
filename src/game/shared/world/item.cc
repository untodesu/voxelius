#include "shared/pch.hh"

#include "shared/world/item.hh"

#include "core/math/crc64.hh"

#include "shared/world/voxel.hh"

world::Item::Item(const Item& source, item_id id) noexcept : Item(source)
{
    m_id = id;
}

void world::Item::set_cached_texture(resource_ptr<TextureGUI> texture) const noexcept
{
    m_cached_texture = std::move(texture);
}

std::uint64_t world::Item::get_checksum(std::uint64_t combine) const
{
    combine = math::crc64(m_name.data(), m_name.size(), combine);
    combine = math::crc64(m_texture.data(), m_texture.size(), combine);

    std::uint32_t id = m_place_voxel ? m_place_voxel->get_id() : NULL_VOXEL_ID;
    combine = math::crc64(&id, sizeof(id), combine);

    return combine;
}

world::ItemBuilder::ItemBuilder(std::string_view name)
{
    set_name(name);
}

void world::ItemBuilder::set_name(std::string_view name)
{
    assert(name.size());

    m_name = name;
}

void world::ItemBuilder::set_texture(std::string_view texture)
{
    m_texture = texture;
}

void world::ItemBuilder::set_place_voxel(const Voxel* place_voxel)
{
    m_place_voxel = place_voxel;
}

std::unique_ptr<world::Item> world::ItemBuilder::build(item_id id) const
{
    return std::make_unique<Item>(*this, id);
}
