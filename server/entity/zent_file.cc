#include "server/pch.hh"

#include "server/entity/zent_file.hh"

#include "core/buffer.hh"
#include "core/utils/crc64.hh"

constexpr static std::uint8_t ZENT_MAGIC_1 = 0x5A;
constexpr static std::uint8_t ZENT_MAGIC_2 = 0x45;
constexpr static std::uint8_t ZENT_MAGIC_3 = 0x4E;
constexpr static std::uint8_t ZENT_MAGIC_4 = 0x54;
constexpr static std::uint32_t ZENT_VERSION = 0x00000001;

ZentFile::ZentFile(std::filesystem::path path) : m_path(std::move(path))
{
    m_offsets.fill(0);
    m_lengths.fill(0);
    m_checksums.fill(0);

    std::error_code ec;
    std::filesystem::create_directories(m_path.parent_path(), ec);

    if(std::filesystem::exists(m_path)) {
        m_stream.open(m_path, std::ios::in | std::ios::out | std::ios::binary);
        m_valid = m_stream.is_open();
        m_valid = m_valid && load_header();
    }
    else {
        m_stream.open(m_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        m_valid = m_stream.is_open();

        if(m_valid) {
            save_header();
        }
    }
}

ZentFile::~ZentFile(void)
{
    if(m_stream.is_open()) {
        m_stream.flush();
        m_stream.close();
    }
}

bool ZentFile::is_empty(void) const
{
    return std::ranges::all_of(m_lengths, [](std::uint64_t length) {
        return length == 0;
    });
}

bool ZentFile::is_valid(void) const
{
    return m_valid;
}

bool ZentFile::read_slot(std::size_t slot, ReadBuffer& buffer)
{
    if(!m_valid || slot >= constant::REGION_VOLUME) {
        return false;
    }

    auto offset = m_offsets[slot];
    auto length = m_lengths[slot];
    auto checksum = m_checksums[slot];

    if(offset == 0 || length == 0) {
        return false;
    }

    std::vector<std::byte> payload;
    payload.resize(length);

    m_stream.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    m_stream.read(reinterpret_cast<char*>(payload.data()), static_cast<std::streamsize>(payload.size()));

    if(!m_stream.good()) {
        return false;
    }

    auto payload_checksum = utils::crc64(payload);

    if(payload_checksum == checksum) {
        buffer.reset(payload);
        return true;
    }

    return false;
}

void ZentFile::write_slot(std::size_t slot, const WriteBuffer& buffer)
{
    if(!m_valid || slot >= constant::REGION_VOLUME) {
        return;
    }

    auto data = buffer.data();
    auto size = buffer.size();

    auto offset = static_cast<std::uint64_t>(m_stream.seekp(0, std::ios::end).tellp());
    auto length = static_cast<std::uint64_t>(size);
    auto checksum = utils::crc64(std::span<const std::byte>(data, size));

    m_stream.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));

    if(!m_stream.good()) {
        return;
    }

    m_offsets[slot] = offset;
    m_lengths[slot] = length;
    m_checksums[slot] = checksum;

    save_header();
}

bool ZentFile::load_header(void)
{
    std::size_t header_size = 0;
    header_size += sizeof(ZENT_MAGIC_1);
    header_size += sizeof(ZENT_MAGIC_2);
    header_size += sizeof(ZENT_MAGIC_3);
    header_size += sizeof(ZENT_MAGIC_4);
    header_size += sizeof(ZENT_VERSION);
    header_size += constant::REGION_VOLUME * sizeof(std::uint64_t);
    header_size += constant::REGION_VOLUME * sizeof(std::uint64_t);
    header_size += constant::REGION_VOLUME * sizeof(std::uint64_t);

    char gambit_character;
    m_stream.seekg(static_cast<std::streamoff>(header_size - 1), std::ios::beg);
    m_stream.read(&gambit_character, sizeof(gambit_character));

    if(!m_stream.good()) {
        return false;
    }

    std::vector<std::byte> header;
    header.resize(header_size);

    m_stream.seekg(0, std::ios::beg);
    m_stream.read(reinterpret_cast<char*>(header.data()), static_cast<std::streamsize>(header.size()));

    if(!m_stream.good()) {
        return false;
    }

    ReadBuffer buffer;
    buffer.reset(header);

    auto magic_1 = buffer.read<std::uint8_t>();
    auto magic_2 = buffer.read<std::uint8_t>();
    auto magic_3 = buffer.read<std::uint8_t>();
    auto magic_4 = buffer.read<std::uint8_t>();
    auto version = buffer.read<std::uint32_t>();

    auto header_valid = true;
    header_valid = header_valid && magic_1 == ZENT_MAGIC_1;
    header_valid = header_valid && magic_2 == ZENT_MAGIC_2;
    header_valid = header_valid && magic_3 == ZENT_MAGIC_3;
    header_valid = header_valid && magic_4 == ZENT_MAGIC_4;
    header_valid = header_valid && version <= ZENT_VERSION;

    if(!header_valid) {
        return false;
    }

    for(std::size_t i = 0; i < constant::REGION_VOLUME; ++i) {
        m_offsets[i] = buffer.read<std::uint64_t>();
        m_lengths[i] = buffer.read<std::uint64_t>();
        m_checksums[i] = buffer.read<std::uint64_t>();
    }

    return true;
}

void ZentFile::save_header(void)
{
    WriteBuffer buffer;

    buffer.write<std::uint8_t>(ZENT_MAGIC_1);
    buffer.write<std::uint8_t>(ZENT_MAGIC_2);
    buffer.write<std::uint8_t>(ZENT_MAGIC_3);
    buffer.write<std::uint8_t>(ZENT_MAGIC_4);
    buffer.write<std::uint32_t>(ZENT_VERSION);

    for(std::size_t i = 0; i < constant::REGION_VOLUME; ++i) {
        buffer.write<std::uint64_t>(m_offsets[i]);
        buffer.write<std::uint64_t>(m_lengths[i]);
        buffer.write<std::uint64_t>(m_checksums[i]);
    }

    m_stream.seekp(0, std::ios::beg);
    buffer.to_stream(m_stream);
}
