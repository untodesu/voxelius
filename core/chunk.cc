#include "core/pch.hh"

#include "core/chunk.hh"

#include "core/buffer.hh"

constexpr static std::size_t HEADER_SIZE = 16;
constexpr static std::size_t TABLE_ENTRY_SIZE = 24;
constexpr static std::size_t CHUNK_ALIGNMENT = 4;

constexpr static std::size_t padding_for(std::size_t size)
{
    return (CHUNK_ALIGNMENT - size % CHUNK_ALIGNMENT) % CHUNK_ALIGNMENT;
}

static std::vector<std::byte> compress_chunk(std::span<const std::byte> data, std::uint32_t compression)
{
    if(compression == CHUNK_COMPRESS_MINIZ) {
        auto bound = mz_compressBound(static_cast<mz_ulong>(data.size()));

        std::vector<std::byte> output;
        output.resize(static_cast<std::size_t>(bound));

        auto input_size = static_cast<mz_ulong>(data.size());
        auto output_size = static_cast<mz_ulong>(output.size());
        auto input_mz = reinterpret_cast<const unsigned char*>(data.data());
        auto output_mz = reinterpret_cast<unsigned char*>(output.data());

        auto result = mz_compress2(output_mz, &output_size, input_mz, input_size, MZ_DEFAULT_LEVEL);
        assert(result == MZ_OK);

        output.resize(static_cast<std::size_t>(output_size));

        return output;
    }

    if(compression == CHUNK_COMPRESS_LZ4) {
        auto bound = LZ4_compressBound(static_cast<int>(data.size()));

        std::vector<std::byte> output;
        output.resize(static_cast<std::size_t>(bound));

        auto input_size = static_cast<int>(data.size());
        auto output_size = static_cast<int>(output.size());
        auto input_lz = reinterpret_cast<const char*>(data.data());
        auto output_lz = reinterpret_cast<char*>(output.data());

        auto written = LZ4_compress_default(input_lz, output_lz, input_size, output_size);
        assert(written > 0);

        output.resize(static_cast<std::size_t>(written));
        return output;
    }

    if(compression == CHUNK_COMPRESS_LZ4HC) {
        auto bound = LZ4_compressBound(static_cast<int>(data.size()));

        std::vector<std::byte> output;
        output.resize(static_cast<std::size_t>(bound));

        auto input_size = static_cast<int>(data.size());
        auto output_size = static_cast<int>(output.size());
        auto input_lz = reinterpret_cast<const char*>(data.data());
        auto output_lz = reinterpret_cast<char*>(output.data());

        auto written = LZ4_compress_HC(input_lz, output_lz, input_size, output_size, LZ4HC_CLEVEL_DEFAULT);
        assert(written > 0);

        output.resize(static_cast<std::size_t>(written));
        return output;
    }

    return std::vector(data.begin(), data.end());
}

static bool decompress_chunk(std::span<const std::byte> data, std::uint32_t compression, std::uint32_t uncompressed_size,
    std::vector<std::byte>& output)
{
    if(compression == CHUNK_COMPRESS_NONE) {
        output.assign(data.begin(), data.end());

        return uncompressed_size == output.size();
    }

    if(compression == CHUNK_COMPRESS_MINIZ) {
        output.resize(uncompressed_size);

        auto output_size = static_cast<mz_ulong>(output.size());
        auto input_size = static_cast<mz_ulong>(data.size());
        auto input_mz = reinterpret_cast<const unsigned char*>(data.data());
        auto output_mz = reinterpret_cast<unsigned char*>(output.data());
        auto result = mz_uncompress(output_mz, &output_size, input_mz, input_size);

        return result == MZ_OK && output_size == uncompressed_size;
    }

    if(compression == CHUNK_COMPRESS_LZ4 || compression == CHUNK_COMPRESS_LZ4HC) {
        output.resize(uncompressed_size);

        auto input_size = static_cast<int>(data.size());
        auto output_size = static_cast<int>(output.size());
        auto input_lz = reinterpret_cast<const char*>(data.data());
        auto output_lz = reinterpret_cast<char*>(output.data());

        auto written = LZ4_decompress_safe(input_lz, output_lz, input_size, output_size);

        return written == static_cast<int>(uncompressed_size);
    }

    return false;
}

std::string ChunkFile::name(std::uint32_t id)
{
    std::string name;
    name.resize(4, char(0x00));

    if(std::endian::native == std::endian::big) {
        id = std::byteswap(id);
    }

    std::memcpy(name.data(), &id, sizeof(id));

    return name;
}

bool ChunkFile::decode(std::span<const std::byte> data, std::uint32_t magic, std::uint32_t format_version)
{
    m_data = {};
    m_chunks.clear();
    m_owned.clear();

    if(data.size() < HEADER_SIZE) {
        return false;
    }

    ReadBuffer header(data.first(HEADER_SIZE));

    auto file_magic = header.read<std::uint32_t>();
    auto file_format_version = header.read<std::uint32_t>();
    auto chunk_count = header.read<std::uint32_t>();
    auto table_offset = header.read<std::uint32_t>();

    if(file_magic != magic || file_format_version != format_version) {
        return false;
    }

    auto table_end = static_cast<std::uint64_t>(table_offset) + static_cast<std::uint64_t>(chunk_count) * TABLE_ENTRY_SIZE;

    if(table_offset < HEADER_SIZE || table_end > data.size()) {
        return false;
    }

    ReadBuffer table;
    table.reset(data.subspan(table_offset, static_cast<std::size_t>(table_end - table_offset)));

    m_chunks.reserve(chunk_count);

    for(std::uint32_t i = 0; i < chunk_count; ++i) {
        ChunkInfo chunk {};
        chunk.id = table.read<std::uint32_t>();
        chunk.version = table.read<std::uint32_t>();
        chunk.offset = table.read<std::uint32_t>();
        chunk.size = table.read<std::uint32_t>();
        chunk.uncompressed_size = table.read<std::uint32_t>();
        chunk.compression = table.read<std::uint32_t>();

        auto chunk_end = static_cast<std::uint64_t>(chunk.offset) + chunk.size;

        if(chunk_end > data.size() || find(chunk.id)) {
            m_chunks.clear();
            return false;
        }

        m_chunks.emplace_back(std::move(chunk));
    }

    m_data = data;

    return true;
}

void ChunkFile::read(const ChunkInfo& chunk, ReadBuffer& buffer) const
{
    read(&chunk, buffer);
}

void ChunkFile::read(const ChunkInfo* chunk, ReadBuffer& buffer) const
{
    assert(chunk);

    if(chunk->compression == CHUNK_COMPRESS_NONE) {
        buffer.reset(m_data.subspan(chunk->offset, chunk->size));
        return;
    }

    auto subspan = m_data.subspan(chunk->offset, chunk->size);
    auto decompressed = std::vector<std::byte> {};
    auto success = decompress_chunk(subspan, chunk->compression, chunk->uncompressed_size, decompressed);
    assert(success);

    buffer.reset(decompressed);
}

const ChunkInfo* ChunkFile::find(std::uint32_t id) const
{
    for(auto& chunk : m_chunks) {
        if(chunk.id == id) {
            return &chunk;
        }
    }

    return nullptr;
}

void ChunkFile::add(std::uint32_t id, std::uint32_t version, std::vector<std::byte> data, std::uint32_t compression)
{
    assert(!find(id));
    assert(m_chunks.size() == m_owned.size()); // mixing decode and add makes no sense

    ChunkInfo chunk {};
    chunk.id = id;
    chunk.version = version;
    chunk.offset = 0;
    chunk.size = static_cast<std::uint32_t>(data.size());
    chunk.uncompressed_size = static_cast<std::uint32_t>(data.size());
    chunk.compression = compression;

    m_chunks.emplace_back(std::move(chunk));
    m_owned.emplace_back(std::move(data));
}

void ChunkFile::encode(std::uint32_t magic, std::uint32_t format_version, WriteBuffer& buffer) const
{
    assert(m_chunks.size() == m_owned.size());

    auto table = m_chunks;

    std::vector<std::vector<std::byte>> encoded;
    encoded.reserve(m_owned.size());

    for(std::size_t i = 0; i < m_owned.size(); ++i) {
        auto compressed = compress_chunk(m_owned[i], table[i].compression);

        if(table[i].compression && compressed.size() >= m_owned[i].size()) {
            table[i].compression = CHUNK_COMPRESS_NONE;
            encoded.emplace_back(m_owned[i]);
        }
        else {
            encoded.emplace_back(std::move(compressed));
        }

        table[i].size = static_cast<std::uint32_t>(encoded.back().size());
    }

    buffer.write<std::uint32_t>(magic);
    buffer.write<std::uint32_t>(format_version);
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(table.size()));
    buffer.write<std::uint32_t>(static_cast<std::uint32_t>(HEADER_SIZE));

    auto offset = HEADER_SIZE + TABLE_ENTRY_SIZE * table.size();

    for(auto& chunk : table) {
        buffer.write<std::uint32_t>(chunk.id);
        buffer.write<std::uint32_t>(chunk.version);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(offset));
        buffer.write<std::uint32_t>(chunk.size);
        buffer.write<std::uint32_t>(chunk.uncompressed_size);
        buffer.write<std::uint32_t>(static_cast<std::uint32_t>(chunk.compression));

        offset += chunk.size + padding_for(chunk.size);
    }

    for(auto& data : encoded) {
        buffer.write_bytes(data);

        for(std::size_t i = 0; i < padding_for(data.size()); ++i) {
            buffer.write<std::byte>(static_cast<std::byte>(0x00));
        }
    }
}
