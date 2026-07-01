#include "core/pch.hh"

#include "core/utils/json.hh"

template<vx::arithmetic T, std::size_t N>
bool utils::parse_json(const JSON_Value* value, Eigen::Vector<T, N>& vector) noexcept
{
    vector.setZero();

    const auto array = json_value_get_array(value);

    if(array == nullptr) {
        return false;
    }

    if(json_array_get_count(array) < N) {
        return false;
    }

    for(std::size_t i = 0; i < N; ++i) {
        const auto element = json_array_get_value(array, i);

        if(element == nullptr) {
            return false;
        }

        const auto string = json_value_get_string(element);

        if(string == nullptr) {
            return false;
        }

        const auto check = std::from_chars(string, string + std::strlen(string), vector[i]);

        if(check.ec != std::errc()) {
            return false;
        }
    }

    return true;
}

template<vx::arithmetic T, std::size_t N>
bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<T, N>& vector) noexcept
{
    const auto value = json_object_get_value(object, name.data());

    if(value == nullptr) {
        return false;
    }

    return parse_json(value, vector);
}

template bool utils::parse_json(const JSON_Value* value, Eigen::Vector<int, 2>& vector) noexcept;
template bool utils::parse_json(const JSON_Value* value, Eigen::Vector<int, 3>& vector) noexcept;
template bool utils::parse_json(const JSON_Value* value, Eigen::Vector<int, 4>& vector) noexcept;

template bool utils::parse_json(const JSON_Value* value, Eigen::Vector<float, 2>& vector) noexcept;
template bool utils::parse_json(const JSON_Value* value, Eigen::Vector<float, 3>& vector) noexcept;
template bool utils::parse_json(const JSON_Value* value, Eigen::Vector<float, 4>& vector) noexcept;

template bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<int, 2>& vector) noexcept;
template bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<int, 3>& vector) noexcept;
template bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<int, 4>& vector) noexcept;

template bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<float, 2>& vector) noexcept;
template bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<float, 3>& vector) noexcept;
template bool utils::parse_json(const JSON_Object* object, std::string_view name, Eigen::Vector<float, 4>& vector) noexcept;
