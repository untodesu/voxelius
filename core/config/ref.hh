#ifndef CBD849A1_0CC9_448F_8C94_69186E185579
#define CBD849A1_0CC9_448F_8C94_69186E185579

#include "core/config/map.hh"

namespace config
{
template<typename T>
class Ref final {
public:
    constexpr Ref(void) noexcept = default;
    constexpr Ref(T default_value) noexcept;

    const T& value(void) const noexcept;
    operator const T&(void) const noexcept;

    void set_value(const T& value) noexcept;
    Ref<T>& operator=(const T& value) noexcept;
    Ref<T>& operator=(const Ref<T>& other) noexcept;

    void bind(Map& map, std::string_view key) noexcept;
    void commit(void) noexcept;

private:
    mutable T m_value {};
    mutable std::uint64_t m_generation { UINT64_MAX };
    slot_type m_slot { null_slot };
    Map* m_map { nullptr };
};
} // namespace config

template<typename T>
constexpr config::Ref<T>::Ref(T default_value) noexcept : m_value(std::move(default_value))
{
    // empty
}

template<typename T>
const T& config::Ref<T>::value(void) const noexcept
{
    if(m_map && m_generation != m_map->generation()) {
        m_value = m_map->value_raw<T>(m_slot).value_or(m_value);
    }

    return m_value;
}

template<typename T>
config::Ref<T>::operator const T&(void) const noexcept
{
    return value();
}

template<typename T>
void config::Ref<T>::set_value(const T& value) noexcept
{
    m_value = value;

    if(m_map && m_slot != null_slot) {
        m_map->set_value_raw<T>(m_slot, value);
        m_generation = m_map->generation();
    }
}

template<typename T>
config::Ref<T>& config::Ref<T>::operator=(const T& value) noexcept
{
    set_value(value);

    return *this;
}

template<typename T>
config::Ref<T>& config::Ref<T>::operator=(const Ref<T>& other) noexcept
{
    set_value(other.value());

    return *this;
}

template<typename T>
void config::Ref<T>::bind(Map& map, std::string_view key) noexcept
{
    m_map = &map;
    m_slot = map.find_or_create_slot(key);
    m_value = map.value_raw<T>(m_slot).value_or(m_value);
    m_generation = map.generation();

    map.set_value_raw<T>(m_slot, m_value);
}

template<typename T>
void config::Ref<T>::commit(void) noexcept
{
    if(m_map && m_slot != null_slot) {
        m_map->set_value_raw<T>(m_slot, m_value);
        m_generation = m_map->generation();
    }
}

#endif /* CBD849A1_0CC9_448F_8C94_69186E185579 */
