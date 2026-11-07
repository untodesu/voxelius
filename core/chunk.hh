#ifndef E7E132C4_2385_429C_96C3_5EC695EEE358
#define E7E132C4_2385_429C_96C3_5EC695EEE358

class ReadBuffer;
class WriteBuffer;

enum chunk_compression : std::uint32_t {
    CHUNK_COMPRESS_NONE = 0,
    CHUNK_COMPRESS_MINIZ,
    CHUNK_COMPRESS_LZ4,
    CHUNK_COMPRESS_LZ4HC,
};

struct ChunkInfo final {
    std::uint32_t id;
    std::uint32_t version;
    std::uint32_t offset;
    std::uint32_t size;              // size on disk, i.e. after compression
    std::uint32_t uncompressed_size; // size once decoded; equal to size when compression == NONE
    std::uint32_t compression;
};

class ChunkFile final {
public:
    static std::string name(std::uint32_t id);

    constexpr std::span<const ChunkInfo> chunks(void) const;

    // The decoded file keeps a span into data, so data must outlive the ChunkFile.
    bool decode(std::span<const std::byte> data, std::uint32_t magic, std::uint32_t format_version);
    void read(const ChunkInfo& chunk, ReadBuffer& buffer) const;
    void read(const ChunkInfo* chunk, ReadBuffer& buffer) const;
    const ChunkInfo* find(std::uint32_t id) const;

    void encode(std::uint32_t magic, std::uint32_t format_version, WriteBuffer& buffer) const;
    void add(std::uint32_t id, std::uint32_t version, std::vector<std::byte> data, std::uint32_t compression = CHUNK_COMPRESS_NONE);

private:
    std::span<const std::byte> m_data {};
    std::vector<ChunkInfo> m_chunks {};
    std::vector<std::vector<std::byte>> m_owned {};
};

constexpr std::span<const ChunkInfo> ChunkFile::chunks(void) const
{
    return m_chunks;
}

#endif /* E7E132C4_2385_429C_96C3_5EC695EEE358 */
