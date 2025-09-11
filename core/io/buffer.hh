#ifndef CORE_IO_BUFFER_HH
#define CORE_IO_BUFFER_HH 1

namespace io
{
class ReadBuffer final {
public:
    ReadBuffer(void) = default;
    explicit ReadBuffer(const ReadBuffer& other);
    explicit ReadBuffer(const void* data, std::size_t size);
    explicit ReadBuffer(const ENetPacket* packet);
    explicit ReadBuffer(PHYSFS_File* file);
    virtual ~ReadBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte* data(void) const;

    void reset(const void* data, std::size_t size);
    void reset(const ENetPacket* packet);
    void reset(PHYSFS_File* file);

    constexpr void rewind(void);
    constexpr bool is_ended(void) const;

    void read(void* buffer, std::size_t size);

    template<typename T>
    T read(void);

    template<typename T>
    ReadBuffer& operator>>(T& value);

private:
    std::vector<std::byte> m_vector;
    std::size_t m_position;
};
} // namespace io

namespace io
{
class WriteBuffer final {
public:
    WriteBuffer(void) = default;
    explicit WriteBuffer(const WriteBuffer& other);
    virtual ~WriteBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte* data(void) const;

    void reset(void);

    void write(const WriteBuffer& other);
    void write(const void* data, std::size_t size);

    template<typename T>
    void write(const T value);

    template<typename T>
    WriteBuffer& operator<<(const T value);

    PHYSFS_File* to_file(const std::string& path, bool append = false) const;
    ENetPacket* to_packet(enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) const;

private:
    std::vector<std::byte> m_vector;
};
} // namespace io

constexpr void io::ReadBuffer::rewind(void)
{
    m_position = 0;
}

constexpr bool io::ReadBuffer::is_ended(void) const
{
    return m_position >= m_vector.size();
}

template<typename T>
io::ReadBuffer& io::ReadBuffer::operator>>(T& value)
{
    value = read<T>();
    return *this;
}

template<typename T>
io::WriteBuffer& io::WriteBuffer::operator<<(const T value)
{
    write<T>(value);
    return *this;
}

#endif // CORE_BUFFER_HH
