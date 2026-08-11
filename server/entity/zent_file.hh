#ifndef E0B9F2A1_5B62_4C4A_9A9D_1C0B3F6D2A11
#define E0B9F2A1_5B62_4C4A_9A9D_1C0B3F6D2A11

#include "server/constant.hh"

class ReadBuffer;
class WriteBuffer;

class ZentFile final {
public:
    explicit ZentFile(std::filesystem::path path);

    ZentFile(const ZentFile& other) = delete;
    ZentFile& operator=(const ZentFile& other) = delete;

    ZentFile(ZentFile&& other) noexcept = default;
    ZentFile& operator=(ZentFile&& other) noexcept = default;

    ~ZentFile(void);

    bool is_empty(void) const;
    bool is_valid(void) const;

    bool read_slot(std::size_t slot, ReadBuffer& buffer);
    void write_slot(std::size_t slot, const WriteBuffer& buffer);

private:
    bool load_header(void);
    void save_header(void);

    std::filesystem::path m_path;
    std::fstream m_stream;
    bool m_valid { false };

    std::array<std::uint64_t, constant::REGION_VOLUME> m_offsets;
    std::array<std::uint64_t, constant::REGION_VOLUME> m_lengths;
    std::array<std::uint64_t, constant::REGION_VOLUME> m_checksums;
};

#endif /* E0B9F2A1_5B62_4C4A_9A9D_1C0B3F6D2A11 */
