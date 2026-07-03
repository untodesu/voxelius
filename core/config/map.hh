#ifndef C8783E57_21EC_40A0_90FF_1576C67F77BE
#define C8783E57_21EC_40A0_90FF_1576C67F77BE

#include "core/concepts.hh"

namespace config
{
using slot_type = std::size_t;
constexpr static slot_type null_slot = std::numeric_limits<slot_type>::max();
} // namespace config

namespace config
{
class Map final {
public:
    Map(void) noexcept = default;

    Map(const Map& other) = delete;
    Map(Map&& other) noexcept = delete;
    Map& operator=(const Map& other) = delete;
    Map& operator=(Map&& other) noexcept = delete;

    constexpr std::uint64_t generation(void) const noexcept;

    std::optional<std::string_view> raw_string(slot_type slot) const noexcept;
    void set_raw_string(slot_type slot, std::string_view value) noexcept;

    template<typename T>
    std::optional<T> value(std::string_view key) const noexcept;
    template<typename T>
    void set_value(std::string_view key, const T& value) noexcept;

    template<typename T>
    std::optional<T> value_raw(slot_type slot) const noexcept;
    template<typename T>
    void set_value_raw(slot_type slot, const T& value) noexcept;

    slot_type find_slot(std::string_view key) const noexcept;
    slot_type find_or_create_slot(std::string_view key) noexcept;

    void purge(void) noexcept;

    void load(std::istream& stream) noexcept;
    void save(std::ostream& stream) const noexcept;

    bool load(std::string_view path) noexcept;
    bool save(std::string_view path) const noexcept;

private:
    std::vector<std::string> m_slots {};
    std::unordered_map<std::string, slot_type> m_index;
    std::uint64_t m_generation { 0 };
};
} // namespace config

constexpr std::uint64_t config::Map::generation(void) const noexcept
{
    return m_generation;
}

template<typename T>
std::optional<T> config::Map::value(std::string_view key) const noexcept
{
    auto slot = find_slot(key);

    if(slot == config::null_slot) {
        return std::nullopt;
    }

    return value_raw<T>(slot);
}

template<typename T>
void config::Map::set_value(std::string_view key, const T& value) noexcept
{
    auto slot = find_or_create_slot(key);

    set_value_raw<T>(slot, value);
}

#endif /* C8783E57_21EC_40A0_90FF_1576C67F77BE */
