// SPDX-License-Identifier: BSD-2-Clause
// Copyright (c) 2025 Kirill Dmitrievich
// File: item.hh
// Description: Item metadata

#ifndef SHARED_WORLD_ITEM_HH
#define SHARED_WORLD_ITEM_HH
#pragma once

#include "core/resource/resource.hh"

#include "shared/types.hh"

// This resource is only defined client-side and
// resource_ptr<TextureGUI> should remain set to null
// anywhere else in the shared and server code
struct TextureGUI;

class Voxel;

class Item {
public:
    Item(void) = default;
    explicit Item(const Item& source, item_id id) noexcept;

    constexpr std::string_view get_name(void) const noexcept;
    constexpr item_id get_id(void) const noexcept;

    constexpr std::string_view get_texture(void) const noexcept;
    constexpr const Voxel* get_place_voxel(void) const noexcept;

    constexpr resource_ptr<TextureGUI>& get_cached_texture(void) const noexcept;
    void set_cached_texture(resource_ptr<TextureGUI> texture) const noexcept;

    std::uint64_t get_checksum(std::uint64_t combine = 0U) const;

protected:
    std::string m_name;
    item_id m_id { NULL_ITEM_ID };

    std::string m_texture;
    const Voxel* m_place_voxel { nullptr };

    mutable resource_ptr<TextureGUI> m_cached_texture; // Client-side only
};

class ItemBuilder final : public Item {
public:
    explicit ItemBuilder(std::string_view name);

    void set_name(std::string_view name);

    void set_texture(std::string_view texture);
    void set_place_voxel(const Voxel* place_voxel);

    std::unique_ptr<Item> build(item_id id) const;
};

constexpr std::string_view Item::get_name(void) const noexcept
{
    return m_name;
}

constexpr item_id Item::get_id(void) const noexcept
{
    return m_id;
}

constexpr std::string_view Item::get_texture(void) const noexcept
{
    return m_texture;
}

constexpr const Voxel* Item::get_place_voxel(void) const noexcept
{
    return m_place_voxel;
}

constexpr resource_ptr<TextureGUI>& Item::get_cached_texture(void) const noexcept
{
    return m_cached_texture;
}

#endif
