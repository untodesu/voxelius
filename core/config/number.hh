#pragma once

#include "core/config/ivalue.hh"

#include "core/math/concepts.hh"

namespace config
{
template<math::Arithmetic T>
class Number : public IValue {
public:
    explicit Number(T default_value = T(0));
    explicit Number(T default_value, T min_value, T max_value);
    virtual ~Number(void) = default;

    virtual void set(std::string_view value) override;
    virtual std::string_view get(void) const override;

    T get_value(void) const;
    void set_value(T value);

    T get_min_value(void) const;
    T get_max_value(void) const;
    void set_limits(T min_value, T max_value);

private:
    T m_value;
    T m_min_value;
    T m_max_value;
    std::string m_string;
};
} // namespace config

namespace config
{
class Int final : public Number<int> {
public:
    using Number<int>::Number;
};

class Float final : public Number<float> {
public:
    using Number<float>::Number;
};

class Unsigned final : public Number<unsigned int> {
public:
    using Number<unsigned int>::Number;
};

class Unsigned64 final : public Number<std::uint64_t> {
public:
    using Number<std::uint64_t>::Number;
};

class SizeType final : public Number<std::size_t> {
public:
    using Number<std::size_t>::Number;
};
} // namespace config

template<math::Arithmetic T>
inline config::Number<T>::Number(T default_value)
{
    m_value = default_value;
    m_min_value = std::numeric_limits<T>::lowest();
    m_max_value = std::numeric_limits<T>::max();
    m_string = std::to_string(default_value);
}

template<math::Arithmetic T>
inline config::Number<T>::Number(T default_value, T min_value, T max_value)
{
    m_value = default_value;
    m_min_value = min_value;
    m_max_value = max_value;
    m_string = std::to_string(default_value);
}

template<math::Arithmetic T>
inline void config::Number<T>::set(std::string_view value)
{
    T parsed_value;
    auto result = std::from_chars(value.data(), value.data() + value.size(), parsed_value);

    if(result.ec == std::errc()) {
        m_value = std::clamp(parsed_value, m_min_value, m_max_value);
        m_string = std::to_string(m_value);
    }
}

template<math::Arithmetic T>
inline std::string_view config::Number<T>::get(void) const
{
    return m_string;
}

template<math::Arithmetic T>
inline T config::Number<T>::get_value(void) const
{
    return m_value;
}

template<math::Arithmetic T>
inline void config::Number<T>::set_value(T value)
{
    m_value = std::clamp(value, m_min_value, m_max_value);
    m_string = std::to_string(m_value);
}

template<math::Arithmetic T>
inline T config::Number<T>::get_min_value(void) const
{
    return m_min_value;
}

template<math::Arithmetic T>
inline T config::Number<T>::get_max_value(void) const
{
    return m_max_value;
}

template<math::Arithmetic T>
inline void config::Number<T>::set_limits(T min_value, T max_value)
{
    m_min_value = min_value;
    m_max_value = max_value;
    m_value = std::clamp(m_value, m_min_value, m_max_value);
    m_string = std::to_string(m_value);
}
