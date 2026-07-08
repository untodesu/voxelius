#ifndef A76309E5_0293_4EA9_95FF_6538BF068CED
#define A76309E5_0293_4EA9_95FF_6538BF068CED

#include "shared/block.hh"
#include "shared/constant.hh"
#include "shared/coord.hh"

class ReadBuffer;
class WriteBuffer;

class BlockStorage final {
public:
    using array8_type = std::array<std::uint8_t, constant::CHUNK_VOLUME>;
    using array16_type = std::array<std::uint16_t, constant::CHUNK_VOLUME>;
    using palette_type = std::vector<std::pair<block_id_type, std::uint16_t>>;

    struct Uniform final {
        block_id_type filler;
    };

    struct Palette8 final {
        array8_type indices;
        palette_type palette;
    };

    struct Palette16 final {
        array16_type indices;
        palette_type palette;
    };

    using variant_type = std::variant<Uniform, Palette8, Palette16>;

    static void serialize(const BlockStorage& storage, WriteBuffer& buffer) noexcept;
    static void deserialize(BlockStorage& storage, ReadBuffer& buffer) noexcept;

    block_id_type get(std::size_t index) const noexcept;
    block_id_type get(const local_pos& pos) const noexcept;

    void set(std::size_t index, block_id_type id) noexcept;
    void set(const local_pos& pos, block_id_type id) noexcept;

    void optimize(void) noexcept;

    std::size_t size(void) const noexcept;

private:
    static std::optional<std::size_t> find_slot(const palette_type& palette, block_id_type id) noexcept;
    static std::size_t add_slot(palette_type& palette, block_id_type id) noexcept;

    static void promote(Palette8& dest, const Uniform& src) noexcept;
    static void promote(Palette16& dest, const Palette8& src) noexcept;

    static void optimize(Palette8& p8, variant_type& dest) noexcept;
    static void optimize(Palette16& p16, variant_type& dest) noexcept;

private:
    variant_type m_variant { Uniform { BLOCK_ID_NULL } };
};

#endif /* A76309E5_0293_4EA9_95FF_6538BF068CED */
