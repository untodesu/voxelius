#include "core/pch.hh"

#include "core/identifier.hh"

#include "core/utils/crc64.hh"
#include "core/utils/string.hh"

Identifier Identifier::from_string(std::string_view string)
{
    return from_string(string, {});
}

Identifier Identifier::from_string(std::string_view string, std::string_view implicit_name_space)
{
    auto trimmed = utils::trim_whitespace<char>(string);

    if(utils::is_whitespace<char>(trimmed)) {
        return {};
    }

    auto separator = trimmed.find(':');
    std::string name_space;
    std::string value;

    if(separator == std::string_view::npos) {
        if(utils::is_whitespace<char>(implicit_name_space)) {
            return Identifier(std::string(trimmed), {}, std::string(trimmed));
        }

        name_space = implicit_name_space;
        value = trimmed;
    }
    else {
        name_space = trimmed.substr(0, separator);
        value = trimmed.substr(separator + 1);
    }

    name_space = utils::trim_whitespace<char>(name_space);
    value = utils::trim_whitespace<char>(value);

    // C++ move semantics are a mess; just shoving std::format into
    // the arguments appears to be undefined behaviour because stuff
    // that goes after the first argument somehow is evaluated first? Then
    // the string is _moved_ into the argument leaving format() inputs empty
    auto full_string = std::format("{}:{}", name_space, value);

    return Identifier(std::move(full_string), std::move(name_space), std::move(value));
}

Identifier Identifier::from_parts(std::string_view name_space, std::string_view value)
{
    if(name_space.empty()) {
        return Identifier(std::string(value), std::string {}, std::string(value));
    }
    else {
        return Identifier(std::format("{}:{}", name_space, value), std::string(name_space), std::string(value));
    }
}

std::string Identifier::as_file_path(std::string_view extension) const
{
    if(m_name_space.empty()) {
        return std::format("{}{}", m_value, extension);
    }
    else {
        return std::format("{}/{}{}", m_name_space, m_value, extension);
    }
}

std::string Identifier::as_file_path(std::string_view subdirectory, std::string_view extension) const
{
    if(m_name_space.empty()) {
        return std::format("{}/{}{}", subdirectory, m_value, extension);
    }
    else {
        return std::format("{}/{}/{}{}", m_name_space, subdirectory, m_value, extension);
    }
}

Identifier::Identifier(std::string full_string, std::string name_space, std::string value)
    : m_full_string(std::move(full_string)), m_name_space(std::move(name_space)), m_value(std::move(value))
{
    // empty
}

std::size_t std::hash<Identifier>::operator()(const Identifier& id) const
{
    return static_cast<std::size_t>(utils::crc64(std::span(id.full_string())));
}
