#ifndef C7541CDE_1304_48E6_BC70_123F817E41E3
#define C7541CDE_1304_48E6_BC70_123F817E41E3

class ReadBuffer final {
public:
    ReadBuffer(void) = default;
    explicit ReadBuffer(const ReadBuffer& other);
    explicit ReadBuffer(const void* data, std::size_t size);
    explicit ReadBuffer(const ENetPacket* packet);
    explicit ReadBuffer(PHYSFS_File* file);
    ~ReadBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte* data(void) const;

    void reset(const void* data, std::size_t size);
    void reset(const ENetPacket* packet);
    void reset(PHYSFS_File* file);
    void reset(std::istream& stream);

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

class WriteBuffer final {
public:
    WriteBuffer(void) = default;
    explicit WriteBuffer(const WriteBuffer& other);
    ~WriteBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte* data(void) const;

    void reset(void);

    void write(const WriteBuffer& other);
    void write(const void* data, std::size_t size);

    template<typename T>
    void write(const T value);

    template<typename T>
    WriteBuffer& operator<<(const T value);

    PHYSFS_File* to_file(std::string_view path, bool append = false) const;
    ENetPacket* to_packet(enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) const;
    void to_stream(std::ostream& stream) const;

private:
    std::vector<std::byte> m_vector;
};

constexpr void ReadBuffer::rewind(void)
{
    m_position = 0;
}

constexpr bool ReadBuffer::is_ended(void) const
{
    return m_position >= m_vector.size();
}

template<typename T>
ReadBuffer& ReadBuffer::operator>>(T& value)
{
    value = read<T>();
    return *this;
}

template<typename T>
WriteBuffer& WriteBuffer::operator<<(const T value)
{
    write<T>(value);
    return *this;
}

#endif /* C7541CDE_1304_48E6_BC70_123F817E41E3 */
