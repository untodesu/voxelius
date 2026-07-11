#include "core/pch.hh"

#include "core/utils/string.hh"

template<vx::char_type T>
constexpr static std::array<T, 7> WHITESPACE_CHARS = { ' ', '\t', '\n', '\v', '\f', '\r', 0 };

template<vx::char_type T>
constexpr static T SINGLE_QUOTE_CHAR = static_cast<T>('\'');
template<vx::char_type T>
constexpr static T DOUBLE_QUOTE_CHAR = static_cast<T>('\"');

template<vx::char_type T>
static bool is_whitespace_char(const T character)
{
    return std::ranges::any_of(WHITESPACE_CHARS<T>, [character](const T whitespace_char) {
        return character == whitespace_char;
    });
}

template<vx::char_type T>
bool utils::is_whitespace(std::basic_string_view<T> string)
{
    auto result = false;
    result = result || string.empty();
    result = result || string.size() == 1 && string[0] == 0;
    result = result || string.find_first_not_of(WHITESPACE_CHARS<T>.data()) == std::basic_string_view<T>::npos;
    return result;
}

template<vx::char_type T>
bool utils::has_whitespace(std::basic_string_view<T> string)
{
    return string.find_first_of(WHITESPACE_CHARS<T>.data()) != std::basic_string_view<T>::npos;
}

template<vx::char_type T>
std::basic_string_view<T> utils::skip_whitespace(std::basic_string_view<T> string)
{
    std::size_t position = 0;

    while(position < string.size() && is_whitespace_char<T>(string[position])) {
        position += 1;
    }

    return string.substr(position);
}

template<vx::char_type T>
std::basic_string_view<T> utils::trim_whitespace(std::basic_string_view<T> string)
{
    auto start = string.find_first_not_of(WHITESPACE_CHARS<T>.data());
    auto end = string.find_last_not_of(WHITESPACE_CHARS<T>.data());

    if(start == std::basic_string_view<T>::npos || end == std::basic_string_view<T>::npos) {
        return {};
    }

    return string.substr(start, end - start + 1);
}

template<vx::char_type T>
std::basic_string_view<T> utils::remove_comments(std::basic_string_view<T> string)
{
    auto comment_position = string.find_first_of(static_cast<T>('#'));

    if(comment_position == std::basic_string_view<T>::npos) {
        return string;
    }

    return string.substr(0, comment_position);
}

template<vx::char_type T>
std::vector<std::basic_string_view<T>> utils::tokenize(std::basic_string_view<T> string)
{
    std::vector<std::basic_string_view<T>> tokens;
    std::size_t position = 0;

    while(position < string.size()) {
        while(position < string.size() && is_whitespace_char<T>(string[position])) {
            position += 1;
        }

        if(position >= string.size()) {
            break;
        }

        T quote_char = 0;

        if(string[position] == SINGLE_QUOTE_CHAR<T> || string[position] == DOUBLE_QUOTE_CHAR<T>) {
            quote_char = string[position];

            position += 1;
        }

        std::size_t token_start = position;

        if(quote_char != 0) {
            while(position < string.size() && string[position] != quote_char) {
                position += 1;
            }

            tokens.push_back(string.substr(token_start, position - token_start));

            if(position < string.size()) {
                position += 1;
            }
        }
        else {
            while(position < string.size() && !is_whitespace_char<T>(string[position])) {
                position += 1;
            }

            tokens.push_back(string.substr(token_start, position - token_start));
        }
    }

    return tokens;
}

template bool utils::is_whitespace<char>(std::basic_string_view<char> string);
template bool utils::is_whitespace<wchar_t>(std::basic_string_view<wchar_t> string);
template bool utils::is_whitespace<char8_t>(std::basic_string_view<char8_t> string);
template bool utils::is_whitespace<char16_t>(std::basic_string_view<char16_t> string);
template bool utils::is_whitespace<char32_t>(std::basic_string_view<char32_t> string);

template bool utils::has_whitespace<char>(std::basic_string_view<char> string);
template bool utils::has_whitespace<wchar_t>(std::basic_string_view<wchar_t> string);
template bool utils::has_whitespace<char8_t>(std::basic_string_view<char8_t> string);
template bool utils::has_whitespace<char16_t>(std::basic_string_view<char16_t> string);
template bool utils::has_whitespace<char32_t>(std::basic_string_view<char32_t> string);

template std::basic_string_view<char> utils::skip_whitespace<char>(std::basic_string_view<char> string);
template std::basic_string_view<wchar_t> utils::skip_whitespace<wchar_t>(std::basic_string_view<wchar_t> string);
template std::basic_string_view<char8_t> utils::skip_whitespace<char8_t>(std::basic_string_view<char8_t> string);
template std::basic_string_view<char16_t> utils::skip_whitespace<char16_t>(std::basic_string_view<char16_t> string);
template std::basic_string_view<char32_t> utils::skip_whitespace<char32_t>(std::basic_string_view<char32_t> string);

template std::basic_string_view<char> utils::trim_whitespace<char>(std::basic_string_view<char> string);
template std::basic_string_view<wchar_t> utils::trim_whitespace<wchar_t>(std::basic_string_view<wchar_t> string);
template std::basic_string_view<char8_t> utils::trim_whitespace<char8_t>(std::basic_string_view<char8_t> string);
template std::basic_string_view<char16_t> utils::trim_whitespace<char16_t>(std::basic_string_view<char16_t> string);
template std::basic_string_view<char32_t> utils::trim_whitespace<char32_t>(std::basic_string_view<char32_t> string);

template std::basic_string_view<char> utils::remove_comments<char>(std::basic_string_view<char> string);
template std::basic_string_view<wchar_t> utils::remove_comments<wchar_t>(std::basic_string_view<wchar_t> string);
template std::basic_string_view<char8_t> utils::remove_comments<char8_t>(std::basic_string_view<char8_t> string);
template std::basic_string_view<char16_t> utils::remove_comments<char16_t>(std::basic_string_view<char16_t> string);
template std::basic_string_view<char32_t> utils::remove_comments<char32_t>(std::basic_string_view<char32_t> string);

template std::vector<std::basic_string_view<char>> utils::tokenize<char>(std::basic_string_view<char> string);
template std::vector<std::basic_string_view<wchar_t>> utils::tokenize<wchar_t>(std::basic_string_view<wchar_t> string);
template std::vector<std::basic_string_view<char8_t>> utils::tokenize<char8_t>(std::basic_string_view<char8_t> string);
template std::vector<std::basic_string_view<char16_t>> utils::tokenize<char16_t>(std::basic_string_view<char16_t> string);
template std::vector<std::basic_string_view<char32_t>> utils::tokenize<char32_t>(std::basic_string_view<char32_t> string);
