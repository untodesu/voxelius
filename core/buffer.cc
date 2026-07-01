#include "core/pch.hh"

#include "core/buffer.hh"

ReadBuffer::ReadBuffer(const ReadBuffer& other)
{
    reset(other.data(), other.size());
}

ReadBuffer::ReadBuffer(const void* data, std::size_t size)
{
    assert(data);

    reset(data, size);
}

ReadBuffer::ReadBuffer(const ENetPacket* packet)
{
    assert(packet);

    reset(packet);
}

ReadBuffer::ReadBuffer(PHYSFS_File* file)
{
    assert(file);

    reset(file);
}

std::size_t ReadBuffer::size(void) const
{
    return m_vector.size();
}

const std::byte* ReadBuffer::data(void) const
{
    return m_vector.data();
}

void ReadBuffer::reset(const void* data, std::size_t size)
{
    assert(data);

    auto bytes = reinterpret_cast<const std::byte*>(data);
    m_vector.assign(bytes, bytes + size);
    m_position = 0;
}

void ReadBuffer::reset(const ENetPacket* packet)
{
    assert(packet);

    auto bytes_ptr = reinterpret_cast<const std::byte*>(packet->data);
    m_vector.assign(bytes_ptr, bytes_ptr + packet->dataLength);
    m_position = 0;
}

void ReadBuffer::reset(PHYSFS_File* file)
{
    assert(file);

    auto size = PHYSFS_fileLength(file);
    assert(size > 0);

    m_vector.resize(static_cast<std::size_t>(size));
    m_position = 0;

    PHYSFS_seek(file, 0);
    PHYSFS_readBytes(file, m_vector.data(), m_vector.size());
}

void ReadBuffer::reset(std::istream& stream)
{
    assert(stream.good());

    stream.seekg(0, std::ios::end);
    auto size = stream.tellg();
    assert(size > 0);

    m_vector.resize(static_cast<std::size_t>(size));
    m_position = 0;

    stream.seekg(0, std::ios::beg);
    stream.read(reinterpret_cast<char*>(m_vector.data()), m_vector.size());
}

template<>
std::byte ReadBuffer::read<std::byte>(void)
{
    if(m_position < m_vector.size()) {
        auto result = m_vector[m_position];
        m_position += 1;
        return result;
    }

    m_position += 1;
    return static_cast<std::byte>(0x00);
}

template<>
std::uint8_t ReadBuffer::read<std::uint8_t>(void)
{
    if((m_position + 1) <= m_vector.size()) {
        auto result = static_cast<std::uint8_t>(m_vector[m_position]);
        m_position += 1;
        return result;
    }

    m_position += 1;
    return 0;
}

template<>
std::uint16_t ReadBuffer::read<std::uint16_t>(void)
{
    if((m_position + 2) <= m_vector.size()) {
        auto result = UINT16_C(0x0000);
        result |= (UINT16_C(0x00FF) & static_cast<std::uint16_t>(m_vector[m_position + 0])) << 8;
        result |= (UINT16_C(0x00FF) & static_cast<std::uint16_t>(m_vector[m_position + 1])) << 0;
        m_position += 2;
        return result;
    }

    m_position += 2;
    return 0;
}

template<>
std::uint32_t ReadBuffer::read<std::uint32_t>(void)
{
    if((m_position + 4) <= m_vector.size()) {
        auto result = UINT32_C(0x00000000);
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 0])) << 24;
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 1])) << 16;
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 2])) << 8;
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 3])) << 0;
        m_position += 4;
        return result;
    }

    m_position += 4;
    return 0;
}

template<>
std::uint64_t ReadBuffer::read<std::uint64_t>(void)
{
    if((m_position + 8) <= m_vector.size()) {
        auto result = UINT64_C(0x0000000000000000);
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 0])) << 56;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 1])) << 48;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 2])) << 40;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 3])) << 32;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 4])) << 24;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 5])) << 16;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 6])) << 8;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 7])) << 0;
        m_position += 8;
        return result;
    }

    m_position += 8;
    return 0;
}

template<>
float ReadBuffer::read<float>(void)
{
    return std::bit_cast<float>(read<std::uint32_t>());
}

template<>
std::int8_t ReadBuffer::read<std::int8_t>(void)
{
    return std::bit_cast<std::int8_t>(read<std::uint8_t>());
}

template<>
std::int16_t ReadBuffer::read<std::int16_t>(void)
{
    return std::bit_cast<std::int16_t>(read<std::uint16_t>());
}

template<>
std::int32_t ReadBuffer::read<std::int32_t>(void)
{
    return std::bit_cast<std::int32_t>(read<std::uint32_t>());
}

template<>
std::int64_t ReadBuffer::read<std::int64_t>(void)
{
    return std::bit_cast<std::int64_t>(read<std::uint64_t>());
}

template<>
std::string ReadBuffer::read<std::string>(void)
{
    std::string result;
    result.resize(read<std::uint16_t>());

    for(std::size_t i = 0; i < result.size(); ++i) {
        if(m_position < m_vector.size()) {
            result[i] = static_cast<char>(m_vector[m_position]);
        }

        m_position += 1;
    }

    return result;
}

void ReadBuffer::read(void* buffer, std::size_t size)
{
    auto bytes = reinterpret_cast<std::byte*>(buffer);
    auto amount_to_read = std::min(size, m_vector.size() - m_position);

    if(amount_to_read > 0) {
        std::copy(m_vector.cbegin() + m_position, m_vector.cbegin() + m_position + amount_to_read, bytes);
    }

    m_position += size;
}

WriteBuffer::WriteBuffer(const WriteBuffer& other)
{
    m_vector = other.m_vector;
}

std::size_t WriteBuffer::size(void) const
{
    return m_vector.size();
}

const std::byte* WriteBuffer::data(void) const
{
    return m_vector.data();
}

void WriteBuffer::reset(void)
{
    m_vector.clear();
}

void WriteBuffer::write(const WriteBuffer& other)
{
    m_vector.insert(m_vector.end(), other.m_vector.begin(), other.m_vector.end());
}

void WriteBuffer::write(const void* data, std::size_t size)
{
    assert(data);

    auto bytes = reinterpret_cast<const std::byte*>(data);
    m_vector.insert(m_vector.end(), bytes, bytes + size);
}

template<>
void WriteBuffer::write<std::byte>(const std::byte value)
{
    m_vector.push_back(value);
}

template<>
void WriteBuffer::write<std::uint8_t>(const std::uint8_t value)
{
    m_vector.push_back(static_cast<std::byte>(value));
}

template<>
void WriteBuffer::write<std::uint16_t>(const std::uint16_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT16_C(0xFF) & ((value & UINT16_C(0xFF00)) >> 8)));
    m_vector.push_back(static_cast<std::byte>(UINT16_C(0xFF) & ((value & UINT16_C(0x00FF)) >> 0)));
}

template<>
void WriteBuffer::write<std::uint32_t>(const std::uint32_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0xFF000000)) >> 24)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x00FF0000)) >> 16)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x0000FF00)) >> 8)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x000000FF)) >> 0)));
}

template<>
void WriteBuffer::write<std::uint64_t>(const std::uint64_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0xFF00000000000000)) >> 56)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x00FF000000000000)) >> 48)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x0000FF0000000000)) >> 40)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x000000FF00000000)) >> 32)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x00000000FF000000)) >> 24)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x0000000000FF0000)) >> 16)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x000000000000FF00)) >> 8)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x00000000000000FF)) >> 0)));
}

template<>
void WriteBuffer::write(const float value)
{
    write(std::bit_cast<std::uint32_t>(value));
}

template<>
void WriteBuffer::write(const std::int8_t value)
{
    write(std::bit_cast<std::uint8_t>(value));
}

template<>
void WriteBuffer::write(const std::int16_t value)
{
    write(std::bit_cast<std::uint16_t>(value));
}

template<>
void WriteBuffer::write(const std::int32_t value)
{
    write(std::bit_cast<std::uint32_t>(value));
}

template<>
void WriteBuffer::write(const std::int64_t value)
{
    write(std::bit_cast<std::uint64_t>(value));
}

template<>
void WriteBuffer::write<std::string_view>(const std::string_view value)
{
    write<std::uint16_t>(static_cast<std::uint16_t>(std::min<std::size_t>(value.size(), UINT16_MAX)));

    for(const auto& character : value) {
        m_vector.push_back(static_cast<std::byte>(character));
    }
}

PHYSFS_File* WriteBuffer::to_file(std::string_view path, bool append) const
{
    std::string path_unfucked(path);
    PHYSFS_File* file = nullptr;

    if(append) {
        file = PHYSFS_openAppend(path_unfucked.c_str());
    }
    else {
        file = PHYSFS_openWrite(path_unfucked.c_str());
    }

    if(file) {
        PHYSFS_writeBytes(file, m_vector.data(), m_vector.size());
    }

    return file;
}

ENetPacket* WriteBuffer::to_packet(enet_uint32 flags) const
{
    return enet_packet_create(m_vector.data(), m_vector.size(), flags);
}

void WriteBuffer::to_stream(std::ostream& stream) const
{
    assert(stream.good());

    stream.write(reinterpret_cast<const char*>(m_vector.data()), m_vector.size());
}
