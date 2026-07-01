#ifndef F04BBF7F_A9D6_40FA_8B14_79BD14AE172B
#define F04BBF7F_A9D6_40FA_8B14_79BD14AE172B

#include "core/config/value.hh"

#include "core/concepts.hh"

namespace config
{
template<vx::arithmetic T>
class Arithmetic final : public IValue {
public:
    constexpr static const T MIN_VALUE = std::numeric_limits<T>::lowest();
    constexpr static const T MAX_VALUE = std::numeric_limits<T>::max();

    explicit Arithmetic(T default_value);
    explicit Arithmetic(T default_value, T min_value, T max_value);
    virtual ~Arithmetic(void) override = default;

    constexpr operator T(void) const noexcept;
    constexpr T arithmetic(void) const noexcept;
    void set_arithmetic(T value) noexcept;

    constexpr T min_value(void) const noexcept;
    void set_min_value(T value) noexcept;

    constexpr T max_value(void) const noexcept;
    void set_max_value(T value) noexcept;

    void set_limits(T min_value, T max_value, bool clamp_value = true) noexcept;

    virtual std::string_view value(void) const noexcept override;
    virtual bool set_value(std::string_view value) noexcept override;

private:
    std::string m_string;
    T m_arithmetic;
    T m_min_value;
    T m_max_value;
};
} // namespace config

namespace config
{
using Int = Arithmetic<int>;
using Float = Arithmetic<float>;
using Double = Arithmetic<double>;
using Unsigned = Arithmetic<unsigned>;
using SizeType = Arithmetic<std::size_t>;
} // namespace config

template<vx::arithmetic T>
config::Arithmetic<T>::Arithmetic(T default_value) : m_arithmetic(default_value), m_min_value(MIN_VALUE), m_max_value(MAX_VALUE)
{
    assert(m_arithmetic >= m_min_value);
    assert(m_arithmetic <= m_max_value);

    m_string = std::to_string(m_arithmetic);
}

template<vx::arithmetic T>
inline config::Arithmetic<T>::Arithmetic(T default_value, T min_value, T max_value)
    : m_arithmetic(default_value), m_min_value(min_value), m_max_value(max_value)
{
    assert(m_max_value >= m_min_value);
    assert(m_arithmetic >= m_min_value);
    assert(m_arithmetic <= m_max_value);

    m_string = std::to_string(m_arithmetic);
}

template<vx::arithmetic T>
inline constexpr config::Arithmetic<T>::operator T(void) const noexcept
{
    return m_arithmetic;
}

template<vx::arithmetic T>
inline constexpr T config::Arithmetic<T>::arithmetic(void) const noexcept
{
    return m_arithmetic;
}

template<vx::arithmetic T>
inline void config::Arithmetic<T>::set_arithmetic(T value) noexcept
{
    m_arithmetic = std::clamp<T>(value, m_min_value, m_max_value);
    m_string = std::to_string(m_arithmetic);
}

template<vx::arithmetic T>
inline constexpr T config::Arithmetic<T>::min_value(void) const noexcept
{
    return m_min_value;
}

template<vx::arithmetic T>
inline void config::Arithmetic<T>::set_min_value(T value) noexcept
{
    assert(value <= m_max_value);

    m_min_value = value;
    m_arithmetic = std::clamp<T>(m_arithmetic, m_min_value, m_max_value);
    m_string = std::to_string(m_arithmetic);
}

template<vx::arithmetic T>
inline constexpr T config::Arithmetic<T>::max_value(void) const noexcept
{
    return m_max_value;
}

template<vx::arithmetic T>
inline void config::Arithmetic<T>::set_max_value(T value) noexcept
{
    assert(value >= m_min_value);

    m_max_value = value;
    m_arithmetic = std::clamp<T>(m_arithmetic, m_min_value, m_max_value);
    m_string = std::to_string(m_arithmetic);
}

template<vx::arithmetic T>
inline void config::Arithmetic<T>::set_limits(T min_value, T max_value, bool clamp_value) noexcept
{
    assert(min_value <= max_value);

    m_min_value = min_value;
    m_max_value = max_value;

    if(clamp_value) {
        m_arithmetic = std::clamp<T>(m_arithmetic, m_min_value, m_max_value);
        m_string = std::to_string(m_arithmetic);
    }
}

template<vx::arithmetic T>
inline std::string_view config::Arithmetic<T>::value(void) const noexcept
{
    return m_string;
}

template<vx::arithmetic T>
inline bool config::Arithmetic<T>::set_value(std::string_view value) noexcept
{
    T new_arithmetic;

    auto check = std::from_chars(value.data(), value.data() + value.size(), new_arithmetic);
    auto is_valid = check.ec == std::errc();

    if(is_valid) {
        m_arithmetic = std::clamp<T>(new_arithmetic, m_min_value, m_max_value);
        m_string = std::to_string(m_arithmetic);
    }

    return is_valid;
}

#endif /* F04BBF7F_A9D6_40FA_8B14_79BD14AE172B */
