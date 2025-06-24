#include "core/pch.hh"

#include "core/buffer.hh"

#include "core/constexpr.hh"

ReadBuffer::ReadBuffer(const void* data, std::size_t size)
{
    reset(data, size);
}

ReadBuffer::ReadBuffer(const ENetPacket* packet)
{
    reset(packet);
}

ReadBuffer::ReadBuffer(PHYSFS_File* file)
{
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
    auto bytes = reinterpret_cast<const std::byte*>(data);
    m_vector.assign(bytes, bytes + size);
    m_position = 0U;
}

void ReadBuffer::reset(const ENetPacket* packet)
{
    auto bytes_ptr = reinterpret_cast<const std::byte*>(packet->data);
    m_vector.assign(bytes_ptr, bytes_ptr + packet->dataLength);
    m_position = 0;
}

void ReadBuffer::reset(PHYSFS_File* file)
{
    m_vector.resize(PHYSFS_fileLength(file));
    m_position = 0;

    PHYSFS_seek(file, 0);
    PHYSFS_readBytes(file, m_vector.data(), m_vector.size());
}

float ReadBuffer::read_FP32(void)
{
    return floathacks::uint32_to_float(read_UI32());
}

std::uint8_t ReadBuffer::read_UI8(void)
{
    if((m_position + 1U) <= m_vector.size()) {
        auto result = static_cast<std::uint8_t>(m_vector[m_position]);
        m_position += 1U;
        return result;
    }

    m_position += 1U;
    return 0;
}

std::uint16_t ReadBuffer::read_UI16(void)
{
    if((m_position + 2U) <= m_vector.size()) {
        auto result = UINT16_C(0x0000);
        result |= static_cast<std::uint16_t>(m_vector[m_position + 0U]) << 8U;
        result |= static_cast<std::uint16_t>(m_vector[m_position + 1U]) << 0U;
        m_position += 2U;
        return result;
    }

    m_position += 2U;
    return 0;
}

std::uint32_t ReadBuffer::read_UI32(void)
{
    if((m_position + 4U) <= m_vector.size()) {
        auto result = UINT32_C(0x00000000);
        result |= static_cast<std::uint32_t>(m_vector[m_position + 0U]) << 24U;
        result |= static_cast<std::uint32_t>(m_vector[m_position + 1U]) << 16U;
        result |= static_cast<std::uint32_t>(m_vector[m_position + 2U]) << 8U;
        result |= static_cast<std::uint32_t>(m_vector[m_position + 3U]) << 0U;
        m_position += 4U;
        return result;
    }

    m_position += 4U;
    return 0;
}

std::uint64_t ReadBuffer::read_UI64(void)
{
    if((m_position + 8U) <= m_vector.size()) {
        auto result = UINT64_C(0x0000000000000000);
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 0U])) << 56U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 1U])) << 48U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 2U])) << 40U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 3U])) << 32U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 4U])) << 24U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 5U])) << 16U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 6U])) << 8U;
        result |= (0x00000000000000FF & static_cast<std::uint64_t>(m_vector[m_position + 7U])) << 0U;
        m_position += 8U;
        return result;
    }

    m_position += 8U;
    return 0;
}

std::string ReadBuffer::read_string(void)
{
    auto size = static_cast<std::size_t>(read_UI16());
    auto result = std::string();

    for(std::size_t i = 0; i < size; ++i) {
        if(m_position < m_vector.size()) {
            result.push_back(static_cast<char>(m_vector[m_position]));
        }

        m_position += 1U;
    }

    return result;
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

void WriteBuffer::write_UI8(std::uint8_t value)
{
    m_vector.push_back(static_cast<std::byte>(value));
}

void WriteBuffer::write_UI16(std::uint16_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT16_C(0xFF) & ((value & UINT16_C(0xFF00)) >> 8U)));
    m_vector.push_back(static_cast<std::byte>(UINT16_C(0xFF) & ((value & UINT16_C(0x00FF)) >> 0U)));
}

void WriteBuffer::write_UI32(std::uint32_t value)
{
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0xFF000000)) >> 24U)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x00FF0000)) >> 16U)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x0000FF00)) >> 8U)));
    m_vector.push_back(static_cast<std::byte>(UINT32_C(0xFF) & ((value & UINT32_C(0x000000FF)) >> 0U)));
}

void WriteBuffer::write_UI64(std::uint64_t value)
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

void WriteBuffer::write_string(const std::string& value)
{
    const std::size_t size = cxpr::min<std::size_t>(UINT16_MAX, value.size());

    write_UI16(static_cast<std::uint16_t>(size));

    for(std::size_t i = 0; i < size; m_vector.push_back(static_cast<std::byte>(value[i++])))
        ;
}

PHYSFS_File* WriteBuffer::to_file(const char* path, bool append) const
{
    if(auto file = (append ? PHYSFS_openAppend(path) : PHYSFS_openWrite(path))) {
        PHYSFS_writeBytes(file, m_vector.data(), m_vector.size());
        return file;
    }

    return nullptr;
}

ENetPacket* WriteBuffer::to_packet(enet_uint32 flags) const
{
    return enet_packet_create(m_vector.data(), m_vector.size(), flags);
}
