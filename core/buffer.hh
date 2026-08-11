#ifndef C7541CDE_1304_48E6_BC70_123F817E41E3
#define C7541CDE_1304_48E6_BC70_123F817E41E3

class ReadBuffer final {
public:
    ReadBuffer(void) = default;
    explicit ReadBuffer(const ReadBuffer& other);
    explicit ReadBuffer(std::span<const std::byte> data);
    explicit ReadBuffer(const ENetPacket* packet);
    explicit ReadBuffer(PHYSFS_File* file);
    ~ReadBuffer(void) = default;

    std::size_t size(void) const;
    const std::byte* data(void) const;

    void reset(std::span<const std::byte> data);
    void reset(const ENetPacket* packet);
    void reset(PHYSFS_File* file);
    void reset(std::istream& stream);

    constexpr void rewind(void);
    constexpr bool is_ended(void) const;

    void read_bytes(std::span<std::byte> buffer);

    template<typename T>
    T read(void);

    template<typename T, std::size_t N>
    Eigen::Vector<T, N> read_vector(void);

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

    void write_buffer(const WriteBuffer& other);
    void write_bytes(std::span<const std::byte> data);

    template<typename T>
    void write(const T value);

    template<typename T, std::size_t N>
    void write_vector(const Eigen::Vector<T, N>& value);

    PHYSFS_File* to_file(std::string_view path, bool append = false) const;
    ENetPacket* to_packet(enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE) const;
    void to_stream(std::ostream& stream) const;

    std::vector<std::byte> take(void);

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

#endif /* C7541CDE_1304_48E6_BC70_123F817E41E3 */
