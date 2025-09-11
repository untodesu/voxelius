#include "core/pch.hh"

#include "core/io/buffer.hh"

#include "core/math/constexpr.hh"

io::ReadBuffer::ReadBuffer(const ReadBuffer& other)
{
    reset(other.data(), other.size());
}

io::ReadBuffer::ReadBuffer(const void* data, std::size_t size)
{
    assert(data);

    reset(data, size);
}

io::ReadBuffer::ReadBuffer(const ENetPacket* packet)
{
    assert(packet);

    reset(packet);
}

io::ReadBuffer::ReadBuffer(PHYSFS_File* file)
{
    assert(file);

    reset(file);
}

std::size_t io::ReadBuffer::size(void) const
{
    return m_vector.size();
}

const std::byte* io::ReadBuffer::data(void) const
{
    return m_vector.data();
}

void io::ReadBuffer::reset(const void* data, std::size_t size)
{
    assert(data);

    auto bytes = reinterpret_cast<const std::byte*>(data);
    m_vector.assign(bytes, bytes + size);
    m_position = 0U;
}

void io::ReadBuffer::reset(const ENetPacket* packet)
{
    assert(packet);

    auto bytes_ptr = reinterpret_cast<const std::byte*>(packet->data);
    m_vector.assign(bytes_ptr, bytes_ptr + packet->dataLength);
    m_position = 0;
}

void io::ReadBuffer::reset(PHYSFS_File* file)
{
    assert(file);

    m_vector.resize(PHYSFS_fileLength(file));
    m_position = 0;

    PHYSFS_seek(file, 0);
    PHYSFS_readBytes(file, m_vector.data(), m_vector.size());
}

template<>
std::byte io::ReadBuffer::read<std::byte>(void)
{
    if(m_position < m_vector.size()) {
        auto result = m_vector[m_position];
        m_position += 1U;
        return result;
    }

    m_position += 1U;
    return static_cast<std::byte>(0x00);
}

template<>
std::uint8_t io::ReadBuffer::read<std::uint8_t>(void)
{
    if((m_position + 1U) <= m_vector.size()) {
        auto result = static_cast<std::uint8_t>(m_vector[m_position]);
        m_position += 1U;
        return result;
    }

    m_position += 1U;
    return 0;
}

template<>
std::uint16_t io::ReadBuffer::read<std::uint16_t>(void)
{
    if((m_position + 2U) <= m_vector.size()) {
        auto result = UINT16_C(0x0000);
        result |= (UINT16_C(0x00FF) & static_cast<std::uint16_t>(m_vector[m_position + 0U])) << 8U;
        result |= (UINT16_C(0x00FF) & static_cast<std::uint16_t>(m_vector[m_position + 1U])) << 0U;
        m_position += 2U;
        return result;
    }

    m_position += 2U;
    return 0;
}

template<>
std::uint32_t io::ReadBuffer::read<std::uint32_t>(void)
{
    if((m_position + 4U) <= m_vector.size()) {
        auto result = UINT32_C(0x00000000);
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 0U])) << 24U;
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 1U])) << 16U;
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 2U])) << 8U;
        result |= (UINT32_C(0x000000FF) & static_cast<std::uint32_t>(m_vector[m_position + 3U])) << 0U;
        m_position += 4U;
        return result;
    }

    m_position += 4U;
    return 0;
}

template<>
std::uint64_t io::ReadBuffer::read<std::uint64_t>(void)
{
    if((m_position + 8U) <= m_vector.size()) {
        auto result = UINT64_C(0x0000000000000000);
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 0U])) << 56U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 1U])) << 48U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 2U])) << 40U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 3U])) << 32U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 4U])) << 24U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 5U])) << 16U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 6U])) << 8U;
        result |= (UINT64_C(0x00000000000000FF) & static_cast<std::uint64_t>(m_vector[m_position + 7U])) << 0U;
        m_position += 8U;
        return result;
    }

    m_position += 8U;
    return 0;
}

template<>
float io::ReadBuffer::read<float>(void)
{
    return std::bit_cast<float>(read<std::uint32_t>());
}

template<>
std::int8_t io::ReadBuffer::read<std::int8_t>(void)
{
    return std::bit_cast<std::int8_t>(read<std::uint8_t>());
}

template<>
std::int16_t io::ReadBuffer::read<std::int16_t>(void)
{
    return std::bit_cast<std::int16_t>(read<std::uint16_t>());
}

template<>
std::int32_t io::ReadBuffer::read<std::int32_t>(void)
{
    return std::bit_cast<std::int32_t>(read<std::uint32_t>());
}

template<>
std::int64_t io::ReadBuffer::read<std::int64_t>(void)
{
    return std::bit_cast<std::int64_t>(read<std::uint64_t>());
}

template<>
std::string io::ReadBuffer::read<std::string>(void)
{
    std::string result;
    result.resize(read<std::uint16_t>());

    for(std::size_t i = 0; i < result.size(); ++i) {
        if(m_position < m_vector.size()) {
            result[i] = static_cast<char>(m_vector[m_position]);
        }

        m_position += 1U;
    }

    return result;
}

void io::ReadBuffer::read(void* buffer, std::size_t size)
{
    auto bytes = reinterpret_cast<std::byte*>(buffer);
    auto amount_to_read = std::min(size, m_vector.size() - m_position);

    if(amount_to_read > 0) {
        std::copy(m_vector.cbegin() + m_position, m_vector.cbegin() + m_position + amount_to_read, bytes);
    }

    m_position += size;
}

io::WriteBuffer::WriteBuffer(const WriteBuffer& other)
{
    m_vector = other.m_vector;
}

std::size_t io::WriteBuffer::size(void) const
{
    return m_vector.size();
}

const std::byte* io::WriteBuffer::data(void) const
{
    return m_vector.data();
}

void io::WriteBuffer::reset(void)
{
    m_vector.clear();
}

void io::WriteBuffer::write(const WriteBuffer& other)
{
    m_vector.insert(m_vector.end(), other.m_vector.begin(), other.m_vector.end());
}

void io::WriteBuffer::write(const void* data, std::size_t size)
{
    assert(data);

    auto bytes = reinterpret_cast<const std::byte*>(data);
    m_vector.insert(m_vector.end(), bytes, bytes + size);
}

template<>
void io::WriteBuffer::write<std::byte>(const std::byte value)
{
    m_vector.push_back(value);
}

template<>
void io::WriteBuffer::write<std::uint8_t>(const std::uint8_t value)
{
    m_vector.push_back(static_cast<std::byte>(value));
}

template<>
void io::WriteBuffer::write<std::uint16_t>(const std::uint16_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT16_C(0xFF) & ((value & UINT16_C(0xFF00)) >> 8U)));
    m_vector.push_back(static_cast<std::byte>(UINT16_C(0xFF) & ((value & UINT16_C(0x00FF)) >> 0U)));
}

template<>
void io::WriteBuffer::write<std::uint32_t>(const std::uint32_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0xFF000000)) >> 24U)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x00FF0000)) >> 16U)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x0000FF00)) >> 8U)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x000000FF)) >> 0U)));
}

template<>
void io::WriteBuffer::write<std::uint64_t>(const std::uint64_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0xFF00000000000000)) >> 56U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x00FF000000000000)) >> 48U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x0000FF0000000000)) >> 40U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x000000FF00000000)) >> 32U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x00000000FF000000)) >> 24U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x0000000000FF0000)) >> 16U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x000000000000FF00)) >> 8U)));
    m_vector.push_back(static_cast<std::byte>(UINT64_C(0xFF) & ((value & UINT64_C(0x00000000000000FF)) >> 0U)));
}

template<>
void io::WriteBuffer::write(const float value)
{
    write(std::bit_cast<std::uint32_t>(value));
}

template<>
void io::WriteBuffer::write(const std::int8_t value)
{
    write(std::bit_cast<std::uint8_t>(value));
}

template<>
void io::WriteBuffer::write(const std::int16_t value)
{
    write(std::bit_cast<std::uint16_t>(value));
}

template<>
void io::WriteBuffer::write(const std::int32_t value)
{
    write(std::bit_cast<std::uint32_t>(value));
}

template<>
void io::WriteBuffer::write(const std::int64_t value)
{
    write(std::bit_cast<std::uint64_t>(value));
}

template<>
void io::WriteBuffer::write<std::string_view>(const std::string_view value)
{
    write<std::uint16_t>(value.size());

    for(const auto& character : value) {
        m_vector.push_back(static_cast<std::byte>(character));
    }
}

PHYSFS_File* io::WriteBuffer::to_file(const std::string& path, bool append) const
{
    PHYSFS_File* file = nullptr;

    if(append) {
        file = PHYSFS_openAppend(path.c_str());
    }
    else {
        file = PHYSFS_openWrite(path.c_str());
    }

    if(file) {
        PHYSFS_writeBytes(file, m_vector.data(), m_vector.size());
    }

    return file;
}

ENetPacket* io::WriteBuffer::to_packet(enet_uint32 flags) const
{
    return enet_packet_create(m_vector.data(), m_vector.size(), flags);
}
