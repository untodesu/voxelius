#ifndef CORE_BUFFER_HH
#define CORE_BUFFER_HH 1

#include "core/floathacks.hh"

class ReadBuffer final {
public:
    explicit ReadBuffer(void) = default;
    explicit ReadBuffer(const void *data, std::size_t size);
    explicit ReadBuffer(const ENetPacket *packet);
    explicit ReadBuffer(PHYSFS_File *file);
    virtual ~ReadBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte *data(void) const;

    void reset(const void *data, std::size_t size);
    void reset(const ENetPacket *packet);
    void reset(PHYSFS_File *file);

    float read_FP32(void);
    std::uint8_t read_UI8(void);
    std::uint16_t read_UI16(void);
    std::uint32_t read_UI32(void);
    std::uint64_t read_UI64(void);
    std::string read_string(void);

    inline std::int8_t read_I8(void);
    inline std::int16_t read_I16(void);
    inline std::int32_t read_I32(void);
    inline std::int64_t read_I64(void);

    inline ReadBuffer &operator>>(float &value);
    inline ReadBuffer &operator>>(std::int8_t &value);
    inline ReadBuffer &operator>>(std::int16_t &value);
    inline ReadBuffer &operator>>(std::int32_t &value);
    inline ReadBuffer &operator>>(std::int64_t &value);
    inline ReadBuffer &operator>>(std::uint8_t &value);
    inline ReadBuffer &operator>>(std::uint16_t &value);
    inline ReadBuffer &operator>>(std::uint32_t &value);
    inline ReadBuffer &operator>>(std::uint64_t &value);
    inline ReadBuffer &operator>>(std::string &value);

private:
    std::vector<std::byte> m_vector;
    std::size_t m_position;
};

class WriteBuffer final {
public:
    explicit WriteBuffer(void) = default;
    virtual ~WriteBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte *data(void) const;

    void reset(void);

    void write_FP32(float value);
    void write_UI8(std::uint8_t value);
    void write_UI16(std::uint16_t value);
    void write_UI32(std::uint32_t value);
    void write_UI64(std::uint64_t value);
    void write_string(const std::string &value);

    inline void write_I8(std::int8_t value);
    inline void write_I16(std::int16_t value);
    inline void write_I32(std::int32_t value);
    inline void write_I64(std::int64_t value);

    inline WriteBuffer &operator<<(float value);
    inline WriteBuffer &operator<<(std::int8_t value);
    inline WriteBuffer &operator<<(std::int16_t value);
    inline WriteBuffer &operator<<(std::int32_t value);
    inline WriteBuffer &operator<<(std::int64_t value);
    inline WriteBuffer &operator<<(std::uint8_t value);
    inline WriteBuffer &operator<<(std::uint16_t value);
    inline WriteBuffer &operator<<(std::uint32_t value);
    inline WriteBuffer &operator<<(std::uint64_t value);
    inline WriteBuffer &operator<<(const std::string &value);

    PHYSFS_File *to_file(const char *path, bool append = false) const;
    ENetPacket *to_packet(enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) const;

private:
    std::vector<std::byte> m_vector;
};

inline std::int8_t ReadBuffer::read_I8(void)
{
    return static_cast<std::int8_t>(read_UI8());
}

inline std::int16_t ReadBuffer::read_I16(void)
{
    return static_cast<std::int16_t>(read_UI16());
}

inline std::int32_t ReadBuffer::read_I32(void)
{
    return static_cast<std::int32_t>(read_UI32());
}

inline std::int64_t ReadBuffer::read_I64(void)
{
    return static_cast<std::int64_t>(read_UI64());
}

inline ReadBuffer &ReadBuffer::operator>>(float &value)
{
    value = read_FP32();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::int8_t &value)
{
    value = read_I8();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::int16_t &value)
{
    value = read_I16();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::int32_t &value)
{
    value = read_I32();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::int64_t &value)
{
    value = read_I64();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::uint8_t &value)
{
    value = read_UI8();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::uint16_t &value)
{
    value = read_UI16();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::uint32_t &value)
{
    value = read_UI32();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::uint64_t &value)
{
    value = read_UI64();
    return *this;
}

inline ReadBuffer &ReadBuffer::operator>>(std::string &value)
{
    value = read_string();
    return *this;
}

inline void WriteBuffer::write_FP32(float value)
{
    write_UI32(floathacks::float_to_uint32(value));
}

inline void WriteBuffer::write_I8(std::int8_t value)
{
    write_UI8(static_cast<std::uint8_t>(value));
}

inline void WriteBuffer::write_I16(std::int16_t value)
{
    write_UI16(static_cast<std::uint16_t>(value));
}

inline void WriteBuffer::write_I32(std::int32_t value)
{
    write_UI32(static_cast<std::uint32_t>(value));
}

inline void WriteBuffer::write_I64(std::int64_t value)
{
    write_UI64(static_cast<std::uint64_t>(value));
}

inline WriteBuffer &WriteBuffer::operator<<(float value)
{
    write_FP32(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::int8_t value)
{
    write_I8(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::int16_t value)
{
    write_I16(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::int32_t value)
{
    write_I32(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::int64_t value)
{
    write_I64(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::uint8_t value)
{
    write_UI8(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::uint16_t value)
{
    write_UI16(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::uint32_t value)
{
    write_UI32(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(std::uint64_t value)
{
    write_UI64(value);
    return *this;
}

inline WriteBuffer &WriteBuffer::operator<<(const std::string &value)
{
    write_string(value);
    return *this;
}

#endif /* CORE_BUFFER_HH */
