#ifndef CLIENT_LANGUAGE_HH
#define CLIENT_LANGUAGE_HH 1
#pragma once

namespace gui
{
struct LanguageInfo final {
    std::string endonym; // Language's self-name
    std::string display; // Display for the settings GUI
    std::string ietf;    // Semi-compliant language abbreviation
};

using LanguageManifest = std::vector<LanguageInfo>;
using LanguageIterator = LanguageManifest::const_iterator;

struct LanguageSetEvent final {
    LanguageIterator new_language;
};
} // namespace gui

namespace gui::language
{
void init(void);
void init_late(void);
} // namespace gui::language

namespace gui::language
{
void set(LanguageIterator new_language);
} // namespace gui::language

namespace gui::language
{
LanguageIterator get_current(void);
LanguageIterator find(std::string_view ietf);
LanguageIterator cbegin(void);
LanguageIterator cend(void);
} // namespace gui::language

namespace gui::language
{
std::string_view resolve(std::string_view key);
std::string resolve_gui(std::string_view key);
} // namespace gui::language

#endif // CLIENT_LANGUAGE_HH
