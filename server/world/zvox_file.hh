#ifndef B2B50862_5694_42F5_9893_F6C24375198D
#define B2B50862_5694_42F5_9893_F6C24375198D

#include "server/constant.hh"

class ReadBuffer;
class WriteBuffer;

class ZvoxFile final {
public:
    explicit ZvoxFile(std::filesystem::path path);

    ZvoxFile(const ZvoxFile& other) = delete;
    ZvoxFile& operator=(const ZvoxFile& other) = delete;

    ZvoxFile(ZvoxFile&& other) noexcept = default;
    ZvoxFile& operator=(ZvoxFile&& other) noexcept = default;

    ~ZvoxFile(void);

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

#endif /* B2B50862_5694_42F5_9893_F6C24375198D */
