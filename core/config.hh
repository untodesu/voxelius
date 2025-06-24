#ifndef CORE_CONFIG_HH
#define CORE_CONFIG_HH 1
#pragma once

class IConfigValue {
public:
    virtual ~IConfigValue(void) = default;
    virtual void set(const char* value) = 0;
    virtual const char* get(void) const = 0;
};

class ConfigBoolean final : public IConfigValue {
public:
    explicit ConfigBoolean(bool default_value = false);
    virtual ~ConfigBoolean(void) = default;

    virtual void set(const char* value) override;
    virtual const char* get(void) const override;

    bool get_value(void) const;
    void set_value(bool value);

private:
    bool m_value;
    std::string m_string;

public:
    static const char* to_string(bool value);
    static bool from_string(const char* value);
};

template<typename T>
class ConfigNumber : public IConfigValue {
    static_assert(std::is_arithmetic_v<T>);

public:
    explicit ConfigNumber(T default_value = T(0));
    explicit ConfigNumber(T default_value, T min_value, T max_value);
    virtual ~ConfigNumber(void) = default;

    virtual void set(const char* value) override;
    virtual const char* get(void) const override;

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

class ConfigInt final : public ConfigNumber<int> {
public:
    using ConfigNumber<int>::ConfigNumber;
};

class ConfigFloat final : public ConfigNumber<float> {
public:
    using ConfigNumber<float>::ConfigNumber;
};

class ConfigUnsigned final : public ConfigNumber<unsigned int> {
public:
    using ConfigNumber<unsigned int>::ConfigNumber;
};

class ConfigUnsigned64 final : public ConfigNumber<std::uint64_t> {
public:
    using ConfigNumber<std::uint64_t>::ConfigNumber;
};

class ConfigSizeType final : public ConfigNumber<std::size_t> {
public:
    using ConfigNumber<std::size_t>::ConfigNumber;
};

class ConfigString final : public IConfigValue {
public:
    explicit ConfigString(const char* default_value);
    virtual ~ConfigString(void) = default;

    virtual void set(const char* value) override;
    virtual const char* get(void) const override;

private:
    std::string m_value;
};

class Config final {
public:
    explicit Config(void) = default;
    virtual ~Config(void) = default;

    void load_cmdline(void);
    bool load_file(const char* path);
    bool save_file(const char* path) const;

    bool set_value(const char* name, const char* value);
    const char* get_value(const char* name) const;

    void add_value(const char* name, IConfigValue& vref);

    const IConfigValue* find(const char* name) const;

private:
    std::unordered_map<std::string, IConfigValue*> m_values;
};

template<typename T>
inline ConfigNumber<T>::ConfigNumber(T default_value)
{
    m_value = default_value;
    m_min_value = std::numeric_limits<T>::min();
    m_max_value = std::numeric_limits<T>::max();
    m_string = std::to_string(default_value);
}

template<typename T>
inline ConfigNumber<T>::ConfigNumber(T default_value, T min_value, T max_value)
{
    m_value = default_value;
    m_min_value = min_value;
    m_max_value = max_value;
    m_string = std::to_string(default_value);
}

template<typename T>
inline void ConfigNumber<T>::set(const char* value)
{
    std::istringstream(value) >> m_value;
    m_value = std::clamp(m_value, m_min_value, m_max_value);
    m_string = std::to_string(m_value);
}

template<typename T>
inline const char* ConfigNumber<T>::get(void) const
{
    return m_string.c_str();
}

template<typename T>
inline T ConfigNumber<T>::get_value(void) const
{
    return m_value;
}

template<typename T>
inline void ConfigNumber<T>::set_value(T value)
{
    m_value = std::clamp(value, m_min_value, m_max_value);
    m_string = std::to_string(m_value);
}

template<typename T>
inline T ConfigNumber<T>::get_min_value(void) const
{
    return m_min_value;
}

template<typename T>
inline T ConfigNumber<T>::get_max_value(void) const
{
    return m_max_value;
}

template<typename T>
inline void ConfigNumber<T>::set_limits(T min_value, T max_value)
{
    m_min_value = min_value;
    m_max_value = max_value;
    m_value = std::clamp(m_value, m_min_value, m_max_value);
    m_string = std::to_string(m_value);
}

#endif /* CORE_CONFIG_HH */
