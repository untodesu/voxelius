#ifndef BD32E72C_9089_4DD8_8319_597C0A144E8D
#define BD32E72C_9089_4DD8_8319_597C0A144E8D

class Identifier final {
public:
    static Identifier from_string(std::string_view string) noexcept;
    static Identifier from_string(std::string_view string, std::string_view implicit_name_space) noexcept;
    static Identifier from_parts(std::string_view name_space, std::string_view value) noexcept;

    Identifier(void) noexcept = default;

    constexpr std::string_view full_string(void) const noexcept;
    constexpr std::string_view name_space(void) const noexcept;
    constexpr std::string_view value(void) const noexcept;

    constexpr bool is_empty(void) const noexcept;
    constexpr bool is_valid(void) const noexcept;

    constexpr bool operator==(const Identifier& other) const noexcept;

    std::string as_file_path(std::string_view extension) const noexcept;
    std::string as_file_path(std::string_view subdirectory, std::string_view extension) const noexcept;

private:
    explicit Identifier(std::string full_string, std::string name_space, std::string value);

    std::string m_full_string;
    std::string m_name_space;
    std::string m_value;
};

template<>
struct std::hash<Identifier> final {
    std::size_t operator()(const Identifier& id) const noexcept;
};

constexpr std::string_view Identifier::full_string(void) const noexcept
{
    return m_full_string;
}

constexpr std::string_view Identifier::name_space(void) const noexcept
{
    return m_name_space;
}

constexpr std::string_view Identifier::value(void) const noexcept
{
    return m_value;
}

constexpr bool Identifier::is_empty(void) const noexcept
{
    return m_full_string.empty();
}

constexpr bool Identifier::is_valid(void) const noexcept
{
    return static_cast<bool>(m_full_string.size());
}

constexpr bool Identifier::operator==(const Identifier& other) const noexcept
{
    return m_full_string == other.m_full_string;
}

#endif /* BD32E72C_9089_4DD8_8319_597C0A144E8D */
