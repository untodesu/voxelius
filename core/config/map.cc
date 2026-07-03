#include "core/pch.hh"

#include "core/config/map.hh"

#include "core/utils/physfs.hh"
#include "core/utils/string.hh"

#include "core/concepts.hh"
#include "core/version.hh"

template<vx::arithmetic T>
static std::optional<T> get_arithmetic(const config::Map* map, config::slot_type slot) noexcept
{
    const auto string = map->raw_string(slot);

    if(string.has_value()) {
        T value;
        auto check = std::from_chars(string->data(), string->data() + string->size(), value);

        if(check.ec == std::errc {}) {
            return value;
        }
    }

    return std::nullopt;
}

template<vx::arithmetic T, std::size_t N>
static std::optional<Eigen::Vector<T, N>> get_vector(const config::Map* map, config::slot_type slot) noexcept
{
    const auto string = map->raw_string(slot);

    if(string.has_value()) {
        std::string_view sv = string.value();
        auto bracket_op = sv.find('[');
        auto bracket_cl = sv.find(']');

        if(bracket_op == std::string_view::npos || bracket_cl == std::string_view::npos || bracket_op > bracket_cl) {
            return std::nullopt;
        }

        std::string_view inner = sv.substr(bracket_op + 1, bracket_cl - bracket_op - 1);
        inner = utils::trim_whitespace<char>(inner);

        auto tokens = utils::tokenize(inner);

        if(tokens.size() >= N) {
            Eigen::Vector<T, N> vector;

            for(std::size_t i = 0; i < N; ++i) {
                auto check = std::from_chars(tokens[i].data(), tokens[i].data() + tokens[i].size(), vector[i]);

                if(check.ec != std::errc {}) {
                    return std::nullopt;
                }
            }

            return vector;
        }
    }

    return std::nullopt;
}

std::optional<std::string_view> config::Map::raw_string(slot_type slot) const noexcept
{
    if(slot >= m_slots.size() || !m_slots[slot].has_value()) {
        return std::nullopt;
    }

    return std::string_view(m_slots[slot].value());
}

void config::Map::set_raw_string(slot_type slot, std::string_view value) noexcept
{
    if(slot < m_slots.size()) {
        m_slots[slot] = std::string(value);
        m_generation += 1;
    }
}

config::slot_type config::Map::find_slot(std::string_view key) const /* E */ noexcept
{
    auto it = m_index.find(std::string(key));

    if(it == m_index.cend()) {
        return config::null_slot;
    }

    return it->second;
}

config::slot_type config::Map::find_or_create_slot(std::string_view key) noexcept
{
    auto it = m_index.find(std::string(key));

    if(it == m_index.cend()) {
        auto slot = m_slots.size();
        m_slots.emplace_back(std::nullopt);
        m_index.insert_or_assign(std::string(key), slot);

        return slot;
    }

    return it->second;
}

void config::Map::purge(void) noexcept
{
    m_slots.clear();
    m_index.clear();
}

void config::Map::load(std::istream& stream) noexcept
{
    std::string line;
    std::string kv_string;

    while(std::getline(stream, line)) {
        kv_string = utils::remove_comments<char>(line);
        kv_string = utils::trim_whitespace<char>(kv_string);

        if(utils::is_whitespace<char>(kv_string)) {
            continue;
        }

        auto separator = kv_string.find('=');

        if(separator == std::string::npos) {
            LOG_WARNING("[stream]: invalid line: '{}'", line);
            continue;
        }

        auto kv_name = kv_string.substr(0, separator);
        auto kv_value = kv_string.substr(separator + 1);

        kv_name = utils::trim_whitespace<char>(kv_name);
        kv_value = utils::trim_whitespace<char>(kv_value);

        set_raw_string(find_or_create_slot(kv_name), kv_value);
    }
}

void config::Map::save(std::ostream& stream) const noexcept
{
    auto curtime = std::time(nullptr);

    stream << "# Voxelius " << version::semantic << " configuration file" << std::endl;
    stream << "# Generated on " << std::asctime(std::localtime(&curtime)) << std::endl << std::endl;

    for(const auto& it : m_index) {
        const auto& slot = m_slots[it.second];

        if(slot.has_value()) {
            stream << it.first << "=";
            stream << slot.value();
            stream << std::endl;
        }
    }
}

bool config::Map::load(std::string_view path) noexcept
{
    std::istringstream stream;

    if(utils::read_file(path, stream)) {
        load(stream);

        return true;
    }

    return false;
}

bool config::Map::save(std::string_view path) const noexcept
{
    std::ostringstream stream;

    save(stream);

    return utils::write_file(path, stream.str());
}

template<>
std::optional<std::string> config::Map::value_raw<std::string>(slot_type slot) const noexcept
{
    auto string = raw_string(slot);

    if(string.has_value()) {
        return std::string(string.value());
    }

    return std::nullopt;
}

template<>
void config::Map::set_value_raw<std::string>(slot_type slot, const std::string& value) noexcept
{
    set_raw_string(slot, value);
}

template<>
std::optional<bool> config::Map::value_raw<bool>(slot_type slot) const noexcept
{
    auto string = raw_string(slot);

    if(string.has_value()) {
        auto is_true = false;
        is_true = is_true || 0 == string.value().compare("true");
        is_true = is_true || 0 == string.value().compare("yes");
        is_true = is_true || 0 == string.value().compare("on");
        is_true = is_true || 0 == string.value().compare("1");

        auto is_false = false;
        is_false = is_false || 0 == string.value().compare("false");
        is_false = is_false || 0 == string.value().compare("off");
        is_false = is_false || 0 == string.value().compare("no");
        is_false = is_false || 0 == string.value().compare("0");

        return is_true && !is_false;
    }

    return std::nullopt;
}

template<>
std::optional<short> config::Map::value_raw<short>(slot_type slot) const noexcept
{
    return get_arithmetic<short>(this, slot);
}

template<>
std::optional<int> config::Map::value_raw<int>(slot_type slot) const noexcept
{
    return get_arithmetic<int>(this, slot);
}

template<>
std::optional<long> config::Map::value_raw<long>(slot_type slot) const noexcept
{
    return get_arithmetic<long>(this, slot);
}

template<>
std::optional<long long> config::Map::value_raw<long long>(slot_type slot) const noexcept
{
    return get_arithmetic<long long>(this, slot);
}

template<>
std::optional<unsigned short> config::Map::value_raw<unsigned short>(slot_type slot) const noexcept
{
    return get_arithmetic<unsigned short>(this, slot);
}

template<>
std::optional<unsigned int> config::Map::value_raw<unsigned int>(slot_type slot) const noexcept
{
    return get_arithmetic<unsigned int>(this, slot);
}

template<>
std::optional<unsigned long> config::Map::value_raw<unsigned long>(slot_type slot) const noexcept
{
    return get_arithmetic<unsigned long>(this, slot);
}

template<>
std::optional<unsigned long long> config::Map::value_raw<unsigned long long>(slot_type slot) const noexcept
{
    return get_arithmetic<unsigned long long>(this, slot);
}

template<>
std::optional<float> config::Map::value_raw<float>(slot_type slot) const noexcept
{
    return get_arithmetic<float>(this, slot);
}

template<>
std::optional<double> config::Map::value_raw<double>(slot_type slot) const noexcept
{
    return get_arithmetic<double>(this, slot);
}

template<>
std::optional<Eigen::Vector2i> config::Map::value_raw<Eigen::Vector2i>(slot_type slot) const noexcept
{
    return get_vector<int, 2>(this, slot);
}

template<>
std::optional<Eigen::Vector3i> config::Map::value_raw<Eigen::Vector3i>(slot_type slot) const noexcept
{
    return get_vector<int, 3>(this, slot);
}

template<>
std::optional<Eigen::Vector4i> config::Map::value_raw<Eigen::Vector4i>(slot_type slot) const noexcept
{
    return get_vector<int, 4>(this, slot);
}

template<>
std::optional<Eigen::Vector2f> config::Map::value_raw<Eigen::Vector2f>(slot_type slot) const noexcept
{
    return get_vector<float, 2>(this, slot);
}

template<>
std::optional<Eigen::Vector3f> config::Map::value_raw<Eigen::Vector3f>(slot_type slot) const noexcept
{
    return get_vector<float, 3>(this, slot);
}

template<>
std::optional<Eigen::Vector4f> config::Map::value_raw<Eigen::Vector4f>(slot_type slot) const noexcept
{
    return get_vector<float, 4>(this, slot);
}

template<>
std::optional<Eigen::Vector2d> config::Map::value_raw<Eigen::Vector2d>(slot_type slot) const noexcept
{
    return get_vector<double, 2>(this, slot);
}

template<>
std::optional<Eigen::Vector3d> config::Map::value_raw<Eigen::Vector3d>(slot_type slot) const noexcept
{
    return get_vector<double, 3>(this, slot);
}

template<>
std::optional<Eigen::Vector4d> config::Map::value_raw<Eigen::Vector4d>(slot_type slot) const noexcept
{
    return get_vector<double, 4>(this, slot);
}

template<>
void config::Map::set_value_raw<bool>(slot_type slot, const bool& value) noexcept
{
    if(value) {
        set_raw_string(slot, "true");
    }
    else {
        set_raw_string(slot, "false");
    }
}

template<>
void config::Map::set_value_raw<short>(slot_type slot, const short& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<int>(slot_type slot, const int& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<long>(slot_type slot, const long& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<long long>(slot_type slot, const long long& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<unsigned short>(slot_type slot, const unsigned short& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<unsigned int>(slot_type slot, const unsigned int& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<unsigned long>(slot_type slot, const unsigned long& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<unsigned long long>(slot_type slot, const unsigned long long& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<float>(slot_type slot, const float& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<double>(slot_type slot, const double& value) noexcept
{
    set_raw_string(slot, std::to_string(value));
}

template<>
void config::Map::set_value_raw<Eigen::Vector2i>(slot_type slot, const Eigen::Vector2i& value) noexcept
{
    set_raw_string(slot, std::format("[{} {}]", value.x(), value.y()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector3i>(slot_type slot, const Eigen::Vector3i& value) noexcept
{
    set_raw_string(slot, std::format("[{} {} {}]", value.x(), value.y(), value.z()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector4i>(slot_type slot, const Eigen::Vector4i& value) noexcept
{
    set_raw_string(slot, std::format("[{} {} {} {}]", value.x(), value.y(), value.z(), value.w()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector2f>(slot_type slot, const Eigen::Vector2f& value) noexcept
{
    set_raw_string(slot, std::format("[{} {}]", value.x(), value.y()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector3f>(slot_type slot, const Eigen::Vector3f& value) noexcept
{
    set_raw_string(slot, std::format("[{} {} {}]", value.x(), value.y(), value.z()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector4f>(slot_type slot, const Eigen::Vector4f& value) noexcept
{
    set_raw_string(slot, std::format("[{} {} {} {}]", value.x(), value.y(), value.z(), value.w()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector2d>(slot_type slot, const Eigen::Vector2d& value) noexcept
{
    set_raw_string(slot, std::format("[{} {}]", value.x(), value.y()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector3d>(slot_type slot, const Eigen::Vector3d& value) noexcept
{
    set_raw_string(slot, std::format("[{} {} {}]", value.x(), value.y(), value.z()));
}

template<>
void config::Map::set_value_raw<Eigen::Vector4d>(slot_type slot, const Eigen::Vector4d& value) noexcept
{
    set_raw_string(slot, std::format("[{} {} {} {}]", value.x(), value.y(), value.z(), value.w()));
}
