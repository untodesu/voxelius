#include "shared/pch.hh"

#include "shared/block_storage.hh"

#include "core/buffer.hh"

#include "shared/utils/coord.hh"

constexpr static std::uint32_t TAG_UNIFORM = 0x85787370;   // UNIF
constexpr static std::uint32_t TAG_PALETTE8 = 0x80657666;  // PALB
constexpr static std::uint32_t TAG_PALETTE16 = 0x80657687; // PALW

std::optional<std::size_t> BlockStorage::find_slot(const palette_type& palette, block_id_type id) noexcept
{
    for(std::size_t i = 0; i < palette.size(); ++i) {
        if(palette[i].first == id) {
            return i;
        }
    }

    return std::nullopt;
}

std::size_t BlockStorage::add_slot(palette_type& palette, block_id_type id) noexcept
{
    palette.emplace_back(id, std::uint16_t { 0 });
    return palette.size() - 1;
}

void BlockStorage::promote(Palette8& dest, const Uniform& src) noexcept
{
    dest.indices.fill(0);
    dest.palette.clear();
    dest.palette.emplace_back(src.filler, static_cast<std::uint16_t>(constant::CHUNK_VOLUME));
}

void BlockStorage::promote(Palette16& dest, const Palette8& src) noexcept
{
    dest.palette = src.palette;

    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        dest.indices[i] = src.indices[i];
    }
}

void BlockStorage::optimize(Palette8& p8, variant_type& dest) noexcept
{
    palette_type compact;
    std::vector<std::uint8_t> remap(p8.palette.size(), 0);

    for(std::size_t i = 0; i < p8.palette.size(); ++i) {
        if(p8.palette[i].second > 0) {
            remap[i] = static_cast<std::uint8_t>(compact.size());
            compact.push_back(p8.palette[i]);
        }
    }

    if(compact.size() <= 1) {
        dest = Uniform { compact.empty() ? BLOCK_ID_NULL : compact.front().first };
        return;
    }

    for(auto& index : p8.indices) {
        index = remap[index];
    }

    p8.palette = std::move(compact);
}

void BlockStorage::optimize(Palette16& p16, variant_type& dest) noexcept
{
    palette_type compact;
    std::vector<std::uint16_t> remap(p16.palette.size(), 0);

    for(std::size_t i = 0; i < p16.palette.size(); ++i) {
        if(p16.palette[i].second > 0) {
            remap[i] = static_cast<std::uint16_t>(compact.size());
            compact.push_back(p16.palette[i]);
        }
    }

    if(compact.size() <= 1) {
        dest = Uniform { compact.empty() ? BLOCK_ID_NULL : compact.front().first };
        return;
    }

    for(auto& index : p16.indices) {
        index = remap[index];
    }

    p16.palette = std::move(compact);

    if(p16.palette.size() <= 256) {
        Palette8 p8;
        p8.palette = p16.palette;

        for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
            p8.indices[i] = static_cast<std::uint8_t>(p16.indices[i]);
        }

        dest = std::move(p8);
    }
}

void BlockStorage::serialize(const BlockStorage& storage, WriteBuffer& buffer) noexcept
{
    if(const auto* uniform = std::get_if<Uniform>(&storage.m_variant)) {
        buffer.write<std::uint32_t>(TAG_UNIFORM);
        buffer.write<block_id_type>(uniform->filler);
        return;
    }

    if(const auto* p8 = std::get_if<Palette8>(&storage.m_variant)) {
        buffer.write<std::uint32_t>(TAG_PALETTE8);
        buffer.write<std::uint16_t>(static_cast<std::uint16_t>(p8->palette.size()));

        for(const auto& [id, refcount] : p8->palette) {
            buffer.write<block_id_type>(id);
        }

        for(const auto index : p8->indices) {
            buffer.write<std::uint8_t>(index);
        }

        return;
    }

    if(const auto* p16 = std::get_if<Palette16>(&storage.m_variant)) {
        buffer.write<std::uint32_t>(TAG_PALETTE16);
        buffer.write<std::uint16_t>(static_cast<std::uint16_t>(p16->palette.size()));

        for(const auto& [id, refcount] : p16->palette) {
            buffer.write<block_id_type>(id);
        }

        for(const auto index : p16->indices) {
            buffer.write<std::uint16_t>(index);
        }

        return;
    }
}

void BlockStorage::deserialize(BlockStorage& storage, ReadBuffer& buffer) noexcept
{
    const auto tag = buffer.read<std::uint32_t>();

    if(tag == TAG_PALETTE8) {
        Palette8 p8;
        const auto count = buffer.read<std::uint16_t>();
        p8.palette.resize(count);

        for(auto& [id, refcount] : p8.palette) {
            id = buffer.read<block_id_type>();
            refcount = 0;
        }

        for(auto& index : p8.indices) {
            index = buffer.read<std::uint8_t>();

            if(index < p8.palette.size()) {
                p8.palette[index].second += 1;
            }
        }

        storage.m_variant = std::move(p8);
        return;
    }

    if(tag == TAG_PALETTE16) {
        Palette16 p16;
        const auto count = buffer.read<std::uint16_t>();
        p16.palette.resize(count);

        for(auto& [id, refcount] : p16.palette) {
            id = buffer.read<block_id_type>();
            refcount = 0;
        }

        for(auto& index : p16.indices) {
            index = buffer.read<std::uint16_t>();

            if(index < p16.palette.size()) {
                p16.palette[index].second += 1;
            }
        }

        storage.m_variant = std::move(p16);
        return;
    }

    // TAG_UNIFORM or unknown/corrupt tag both fall back to a null-filled chunk
    storage.m_variant = Uniform { buffer.read<block_id_type>() };
}

block_id_type BlockStorage::get(std::size_t index) const noexcept
{
    if(index >= constant::CHUNK_VOLUME) {
        return BLOCK_ID_NULL;
    }

    if(const auto* uniform = std::get_if<Uniform>(&m_variant)) {
        return uniform->filler;
    }

    if(const auto* p8 = std::get_if<Palette8>(&m_variant)) {
        return p8->palette[p8->indices[index]].first;
    }

    if(const auto* p16 = std::get_if<Palette16>(&m_variant)) {
        return p16->palette[p16->indices[index]].first;
    }

    return BLOCK_ID_NULL;
}

block_id_type BlockStorage::get(const local_pos& pos) const noexcept
{
    return get(utils::to_index(pos));
}

void BlockStorage::set(std::size_t index, block_id_type id) noexcept
{
    if(index >= constant::CHUNK_VOLUME) {
        return;
    }

    if(const auto* uniform = std::get_if<Uniform>(&m_variant)) {
        if(uniform->filler == id) {
            return;
        }

        Palette8 p8;
        promote(p8, *uniform);
        m_variant = std::move(p8);
        set(index, id);
        return;
    }

    if(auto* p8 = std::get_if<Palette8>(&m_variant)) {
        auto slot = find_slot(p8->palette, id);

        if(!slot && p8->palette.size() >= 256) {
            Palette16 p16;
            promote(p16, *p8);
            m_variant = std::move(p16);
            set(index, id);
            return;
        }

        const std::size_t new_slot = slot ? *slot : add_slot(p8->palette, id);
        p8->palette[p8->indices[index]].second -= 1;
        p8->indices[index] = static_cast<std::uint8_t>(new_slot);
        p8->palette[new_slot].second += 1;
        return;
    }

    if(auto* p16 = std::get_if<Palette16>(&m_variant)) {
        auto slot = find_slot(p16->palette, id);
        const std::size_t new_slot = slot ? *slot : add_slot(p16->palette, id);
        p16->palette[p16->indices[index]].second -= 1;
        p16->indices[index] = static_cast<std::uint16_t>(new_slot);
        p16->palette[new_slot].second += 1;
        return;
    }
}

void BlockStorage::set(const local_pos& pos, block_id_type id) noexcept
{
    set(utils::to_index(pos), id);
}

void BlockStorage::optimize(void) noexcept
{
    if(auto* p8 = std::get_if<Palette8>(&m_variant)) {
        optimize(*p8, m_variant);
        return;
    }

    if(auto* p16 = std::get_if<Palette16>(&m_variant)) {
        optimize(*p16, m_variant);
        return;
    }
}

std::size_t BlockStorage::size(void) const noexcept
{
    if(std::holds_alternative<Uniform>(m_variant)) {
        return sizeof(Uniform);
    }

    if(const auto* p8 = std::get_if<Palette8>(&m_variant)) {
        return p8->indices.size() * sizeof(std::uint8_t) + p8->palette.size() * sizeof(palette_type::value_type);
    }

    if(const auto* p16 = std::get_if<Palette16>(&m_variant)) {
        return p16->indices.size() * sizeof(std::uint16_t) + p16->palette.size() * sizeof(palette_type::value_type);
    }

    return 0;
}
