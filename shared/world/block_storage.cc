#include "shared/pch.hh"

#include "shared/world/block_storage.hh"

#include "core/buffer.hh"

#include "shared/utils/coord.hh"

constexpr static std::uint32_t TAG_UNIFORM = 0x85787370;   // UNIF
constexpr static std::uint32_t TAG_PALETTE8 = 0x80657666;  // PALB
constexpr static std::uint32_t TAG_PALETTE16 = 0x80657687; // PALW

void BlockStorage::serialize(const BlockStorage& storage, WriteBuffer& buffer)
{
    if(const auto uniform = std::get_if<Uniform>(&storage.m_variant)) {
        buffer.write<std::uint32_t>(TAG_UNIFORM);
        serialize(uniform, buffer);
        return;
    }

    if(const auto p8 = std::get_if<Palette8>(&storage.m_variant)) {
        buffer.write<std::uint32_t>(TAG_PALETTE8);
        serialize(p8, buffer);
        return;
    }

    if(const auto p16 = std::get_if<Palette16>(&storage.m_variant)) {
        buffer.write<std::uint32_t>(TAG_PALETTE16);
        serialize(p16, buffer);
        return;
    }
}

void BlockStorage::deserialize(BlockStorage& storage, ReadBuffer& buffer)
{
    const auto tag = buffer.read<std::uint32_t>();

    if(tag == TAG_PALETTE8) {
        Palette8 p8;
        deserialize(p8, buffer);
        storage.m_variant = std::move(p8);
        return;
    }

    if(tag == TAG_PALETTE16) {
        Palette16 p16;
        deserialize(p16, buffer);
        storage.m_variant = std::move(p16);
        return;
    }

    if(tag == TAG_UNIFORM) {
        Uniform uniform;
        deserialize(uniform, buffer);
        storage.m_variant = std::move(uniform);
        return;
    }
}

void BlockStorage::serialize(const Uniform* uniform, WriteBuffer& buffer)
{
    buffer.write<std::uint32_t>(uniform->filler);
}

void BlockStorage::serialize(const Palette8* p8, WriteBuffer& buffer)
{
    buffer.write<std::uint16_t>(static_cast<std::uint16_t>(p8->palette.size()));

    for(const auto& [id, refcount] : p8->palette) {
        buffer.write<std::uint32_t>(id);
        buffer.write<std::uint16_t>(refcount);
    }

    for(const auto index : p8->indices) {
        buffer.write<std::uint8_t>(index);
    }
}

void BlockStorage::serialize(const Palette16* p16, WriteBuffer& buffer)
{
    buffer.write<std::uint16_t>(static_cast<std::uint16_t>(p16->palette.size()));

    for(const auto& [id, refcount] : p16->palette) {
        buffer.write<std::uint32_t>(id);
        buffer.write<std::uint16_t>(refcount);
    }

    for(const auto index : p16->indices) {
        buffer.write<std::uint16_t>(index);
    }
}

void BlockStorage::deserialize(Uniform& uniform, ReadBuffer& buffer)
{
    uniform.filler = buffer.read<std::uint32_t>();
}

void BlockStorage::deserialize(Palette8& p8, ReadBuffer& buffer)
{
    p8.palette.resize(buffer.read<std::uint16_t>());

    for(auto& [id, refcount] : p8.palette) {
        id = buffer.read<std::uint32_t>();
        refcount = buffer.read<std::uint16_t>();
    }

    if(p8.palette.empty()) {
        p8.palette.emplace_back(BLOCK_ID_NULL, static_cast<std::uint16_t>(constant::CHUNK_VOLUME));
    }

    const auto max_slot = static_cast<std::uint8_t>(p8.palette.size() - 1);

    for(auto& index : p8.indices) {
        index = std::min(buffer.read<std::uint8_t>(), max_slot);
    }
}

void BlockStorage::deserialize(Palette16& p16, ReadBuffer& buffer)
{
    p16.palette.resize(buffer.read<std::uint16_t>());

    for(auto& [id, refcount] : p16.palette) {
        id = buffer.read<std::uint32_t>();
        refcount = buffer.read<std::uint16_t>();
    }

    if(p16.palette.empty()) {
        p16.palette.emplace_back(BLOCK_ID_NULL, static_cast<std::uint16_t>(constant::CHUNK_VOLUME));
    }

    const auto max_slot = static_cast<std::uint16_t>(p16.palette.size() - 1);

    for(auto& index : p16.indices) {
        index = std::min(buffer.read<std::uint16_t>(), max_slot);
    }
}

std::optional<std::size_t> BlockStorage::find_slot(const palette_type& palette, block_id_type id)
{
    for(std::size_t i = 0; i < palette.size(); ++i) {
        if(palette[i].first == id) {
            return i;
        }
    }

    return std::nullopt;
}

std::size_t BlockStorage::add_slot(palette_type& palette, block_id_type id)
{
    auto slot = palette.size();
    palette.emplace_back(id, UINT16_C(0));
    return slot;
}

void BlockStorage::promote(Palette8& dest, const Uniform& src)
{
    dest.indices.fill(0);
    dest.palette.clear();
    dest.palette.emplace_back(src.filler, static_cast<std::uint16_t>(constant::CHUNK_VOLUME));
}

void BlockStorage::promote(Palette16& dest, const Palette8& src)
{
    dest.palette = src.palette;

    for(std::size_t i = 0; i < constant::CHUNK_VOLUME; ++i) {
        dest.indices[i] = src.indices[i];
    }
}

void BlockStorage::optimize(Palette8& p8, variant_type& dest)
{
    palette_type compact;
    std::vector<std::uint8_t> remap;
    remap.resize(p8.palette.size(), 0);

    for(std::size_t i = 0; i < p8.palette.size(); ++i) {
        if(p8.palette[i].second > 0) {
            remap[i] = static_cast<std::uint8_t>(compact.size());
            compact.push_back(p8.palette[i]);
        }
    }

    if(compact.empty()) {
        dest = Uniform { BLOCK_ID_NULL };
        return;
    }

    if(1 == compact.size()) {
        dest = Uniform { compact[0].first };
        return;
    }

    for(auto& index : p8.indices) {
        index = remap[index];
    }

    p8.palette = std::move(compact);
}

void BlockStorage::optimize(Palette16& p16, variant_type& dest)
{
    palette_type compact;
    std::vector<std::uint16_t> remap;
    remap.resize(p16.palette.size(), 0);

    for(std::size_t i = 0; i < p16.palette.size(); ++i) {
        if(p16.palette[i].second > 0) {
            remap[i] = static_cast<std::uint16_t>(compact.size());
            compact.push_back(p16.palette[i]);
        }
    }

    if(compact.empty()) {
        dest = Uniform { BLOCK_ID_NULL };
        return;
    }

    if(1 == compact.size()) {
        dest = Uniform { compact[0].first };
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

block_id_type BlockStorage::get(std::size_t index) const
{
    if(index >= constant::CHUNK_VOLUME) {
        return BLOCK_ID_NULL;
    }

    if(const auto uniform = std::get_if<Uniform>(&m_variant)) {
        return uniform->filler;
    }

    if(const auto p8 = std::get_if<Palette8>(&m_variant)) {
        return p8->palette[p8->indices[index]].first;
    }

    if(const auto p16 = std::get_if<Palette16>(&m_variant)) {
        return p16->palette[p16->indices[index]].first;
    }

    return BLOCK_ID_NULL;
}

block_id_type BlockStorage::get(const LocalPos& pos) const
{
    return get(utils::to_index(pos));
}

void BlockStorage::set(std::size_t index, block_id_type id)
{
    if(index >= constant::CHUNK_VOLUME) {
        return;
    }

    if(const auto uniform = std::get_if<Uniform>(&m_variant)) {
        if(uniform->filler == id) {
            return;
        }

        Palette8 p8;
        promote(p8, *uniform);
        m_variant = std::move(p8);

        set(index, id);

        return;
    }

    if(auto p8 = std::get_if<Palette8>(&m_variant)) {
        auto slot = find_slot(p8->palette, id);

        if(!slot.has_value() && p8->palette.size() >= 256) {
            Palette16 p16;
            promote(p16, *p8);
            m_variant = std::move(p16);

            set(index, id);

            return;
        }

        auto new_slot = slot.has_value() ? slot.value() : add_slot(p8->palette, id);

        p8->palette[p8->indices[index]].second -= 1;
        p8->indices[index] = static_cast<std::uint8_t>(new_slot);
        p8->palette[new_slot].second += 1;

        return;
    }

    if(auto p16 = std::get_if<Palette16>(&m_variant)) {
        auto slot = find_slot(p16->palette, id);
        auto new_slot = slot.has_value() ? slot.value() : add_slot(p16->palette, id);

        p16->palette[p16->indices[index]].second -= 1;
        p16->indices[index] = static_cast<std::uint16_t>(new_slot);
        p16->palette[new_slot].second += 1;

        return;
    }
}

void BlockStorage::set(const LocalPos& pos, block_id_type id)
{
    set(utils::to_index(pos), id);
}

void BlockStorage::fill(block_id_type id)
{
    Uniform uniform {};
    uniform.filler = id;
    m_variant = std::move(uniform);
}

void BlockStorage::optimize(void)
{
    if(auto p8 = std::get_if<Palette8>(&m_variant)) {
        optimize(*p8, m_variant);
        return;
    }

    if(auto p16 = std::get_if<Palette16>(&m_variant)) {
        optimize(*p16, m_variant);
        return;
    }
}

std::size_t BlockStorage::size(void) const
{
    if(std::holds_alternative<Uniform>(m_variant)) {
        return sizeof(Uniform);
    }

    if(const auto p8 = std::get_if<Palette8>(&m_variant)) {
        std::size_t result = sizeof(Palette8);
        result += p8->palette.size() * sizeof(palette_type::value_type);
        return result;
    }

    if(const auto p16 = std::get_if<Palette16>(&m_variant)) {
        std::size_t result = sizeof(Palette16);
        result += p16->palette.size() * sizeof(palette_type::value_type);
        return result;
    }

    return 0;
}
