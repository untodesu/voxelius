#pragma once

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

namespace language
{
void init(void);
void init_late(void);
} // namespace language

namespace language
{
void set(LanguageIterator new_language);
} // namespace language

namespace language
{
LanguageIterator get_current(void);
LanguageIterator find(std::string_view ietf);
LanguageIterator cbegin(void);
LanguageIterator cend(void);
} // namespace language

namespace language
{
std::string_view resolve(std::string_view key);
std::string resolve_gui(std::string_view key);
} // namespace language
