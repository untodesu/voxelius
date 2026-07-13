#ifndef CBD849A1_0CC9_448F_8C94_69186E185579
#define CBD849A1_0CC9_448F_8C94_69186E185579

#include "core/config/map.hh"

namespace config
{
template<typename T>
class Ref final {
public:
    constexpr Ref(void) = default;
    constexpr Ref(T default_value);

    const T& value(void) const;
    operator const T&(void) const;

    constexpr bool dirty(void) const;

    void set_value(const T& value);
    Ref<T>& operator=(const T& value);
    Ref<T>& operator=(const Ref<T>& other);

    void bind(Map& map, std::string_view key);
    void commit(void);

private:
    mutable T m_value {};
    mutable std::uint64_t m_generation { UINT64_MAX };
    map_slot_type m_slot { NULL_SLOT };
    Map* m_map { nullptr };
};
} // namespace config

template<typename T>
constexpr config::Ref<T>::Ref(T default_value) : m_value(std::move(default_value))
{
    // empty
}

template<typename T>
const T& config::Ref<T>::value(void) const
{
    if(dirty()) {
        m_value = m_map->value_raw<T>(m_slot).value_or(m_value);
        m_generation = m_map->generation(m_slot);
    }

    return m_value;
}

template<typename T>
config::Ref<T>::operator const T&(void) const
{
    return value();
}

template<typename T>
constexpr bool config::Ref<T>::dirty(void) const
{
    if(m_map == nullptr || m_generation == m_map->generation(m_slot)) {
        return false;
    }

    return true;
}

template<typename T>
void config::Ref<T>::set_value(const T& value)
{
    m_value = value;

    if(m_map && m_slot != NULL_SLOT) {
        m_map->set_value_raw<T>(m_slot, value);
    }
}

template<typename T>
config::Ref<T>& config::Ref<T>::operator=(const T& value)
{
    set_value(value);

    return *this;
}

template<typename T>
config::Ref<T>& config::Ref<T>::operator=(const Ref<T>& other)
{
    set_value(other.value());

    return *this;
}

template<typename T>
void config::Ref<T>::bind(Map& map, std::string_view key)
{
    m_map = &map;
    m_slot = map.find_or_create_slot(key);
    m_value = map.value_raw<T>(m_slot).value_or(m_value);
    m_generation = map.generation(m_slot);

    map.set_value_raw<T>(m_slot, m_value);
}

template<typename T>
void config::Ref<T>::commit(void)
{
    if(m_map && m_slot != NULL_SLOT) {
        m_map->set_value_raw<T>(m_slot, m_value);
        m_generation = m_map->generation(m_slot);
    }
}

#endif /* CBD849A1_0CC9_448F_8C94_69186E185579 */
