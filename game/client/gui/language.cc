#include "client/pch.hh"

#include "client/gui/language.hh"

#include "core/config/string.hh"

#include "core/io/config_map.hh"

#include "client/gui/settings.hh"

#include "client/globals.hh"

constexpr static std::string_view DEFAULT_LANGUAGE = "en_US";

// Available languages are kept in a special manifest file which
// is essentially a key-value map of semi-IETF-compliant language tags
// and the language's endonym; after reading the manifest, the translation
// system knows what language it can load and will act accordingly
constexpr static std::string_view MANIFEST_PATH = "lang/manifest.json";

static gui::LanguageManifest manifest;
static gui::LanguageIterator current_language;
static std::unordered_map<std::string, std::string> language_map;
static std::unordered_map<std::string, gui::LanguageIterator> ietf_map;
static config::String config_language(DEFAULT_LANGUAGE);

static void send_language_event(gui::LanguageIterator new_language)
{
    gui::LanguageSetEvent event;
    event.new_language = new_language;
    globals::dispatcher.trigger(event);
}

void gui::language::init(void)
{
    globals::client_config.add_value("language", config_language);

    settings::add_language_select(0, settings_location::GENERAL, "language");

    auto file = PHYSFS_openRead(std::string(MANIFEST_PATH).c_str());

    if(file == nullptr) {
        spdlog::critical("language: {}: {}", MANIFEST_PATH, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        std::terminate();
    }

    auto source = std::string(PHYSFS_fileLength(file), char(0x00));
    PHYSFS_readBytes(file, source.data(), source.size());
    PHYSFS_close(file);

    auto jsonv = json_parse_string(source.c_str());
    const auto json = json_value_get_object(jsonv);
    const auto count = json_object_get_count(json);

    if((jsonv == nullptr) || (json == nullptr) || (count == 0)) {
        spdlog::critical("language: {}: parse error", MANIFEST_PATH);
        json_value_free(jsonv);
        std::terminate();
    }

    for(std::size_t i = 0; i < count; ++i) {
        const auto ietf = json_object_get_name(json, i);
        const auto value = json_object_get_value_at(json, i);
        const auto endonym = json_value_get_string(value);

        if(ietf && endonym) {
            LanguageInfo info;
            info.ietf = std::string(ietf);
            info.endonym = std::string(endonym);
            info.display = std::format("{} ({})", endonym, ietf);
            manifest.push_back(info);
        }
    }

    for(auto it = manifest.cbegin(); it != manifest.cend(); ++it) {
        ietf_map.emplace(it->ietf, it);
    }

    json_value_free(jsonv);

    // This is temporary! This value will
    // be overriden in init_late after the
    // config system updates config_language
    current_language = manifest.cend();
}

void gui::language::init_late(void)
{
    auto user_language = ietf_map.find(config_language.get_value());

    if(user_language != ietf_map.cend()) {
        gui::language::set(user_language->second);
        return;
    }

    auto fallback = ietf_map.find(std::string(DEFAULT_LANGUAGE));

    if(fallback != ietf_map.cend()) {
        gui::language::set(fallback->second);
        return;
    }

    spdlog::critical("language: we're doomed!");
    spdlog::critical("language: {} doesn't exist!", DEFAULT_LANGUAGE);
    std::terminate();
}

void gui::language::set(LanguageIterator new_language)
{
    if(new_language != manifest.cend()) {
        auto path = std::format("lang/lang.{}.json", new_language->ietf);

        auto file = PHYSFS_openRead(path.c_str());

        if(file == nullptr) {
            spdlog::warn("language: {}: {}", path, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
            send_language_event(new_language);
            return;
        }

        auto source = std::string(PHYSFS_fileLength(file), char(0x00));
        PHYSFS_readBytes(file, source.data(), source.size());
        PHYSFS_close(file);

        auto jsonv = json_parse_string(source.c_str());
        const auto json = json_value_get_object(jsonv);
        const auto count = json_object_get_count(json);

        if((jsonv == nullptr) || (json == nullptr) || (count == 0)) {
            spdlog::warn("language: {}: parse error", path);
            send_language_event(new_language);
            json_value_free(jsonv);
            return;
        }

        language_map.clear();

        for(size_t i = 0; i < count; ++i) {
            const auto key = json_object_get_name(json, i);
            const auto value = json_object_get_value_at(json, i);
            const auto value_str = json_value_get_string(value);

            if(key && value_str) {
                language_map.emplace(key, value_str);
                continue;
            }
        }

        json_value_free(jsonv);

        current_language = new_language;
        config_language.set(new_language->ietf.c_str());
    }

    send_language_event(new_language);
}

gui::LanguageIterator gui::language::get_current(void)
{
    return current_language;
}

gui::LanguageIterator gui::language::find(std::string_view ietf)
{
    const auto it = ietf_map.find(std::string(ietf));
    if(it != ietf_map.cend()) {
        return it->second;
    }
    else {
        return manifest.cend();
    }
}

gui::LanguageIterator gui::language::cbegin(void)
{
    return manifest.cbegin();
}

gui::LanguageIterator gui::language::cend(void)
{
    return manifest.cend();
}

std::string_view gui::language::resolve(std::string_view key)
{
    const auto it = language_map.find(std::string(key));

    if(it != language_map.cend()) {
        return it->second;
    }

    return key;
}

std::string gui::language::resolve_gui(std::string_view key)
{
    // We need window tags to retain their hierarchy when a language
    // dynamically changes; ImGui allows to provide hidden unique identifiers
    // to GUI primitives that have their name change dynamically, so we're using this
    return std::format("{}###{}", gui::language::resolve(key), key);
}
