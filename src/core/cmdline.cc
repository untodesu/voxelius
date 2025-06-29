#include "core/pch.hh"

#include "core/cmdline.hh"

// Valid options always start with OPTION_PREFIX, can contain
// a bunch of OPTION_PREFIX'es inside and never end with one
constexpr static char OPTION_PREFIX = '-';

static std::unordered_map<std::string, std::string> options;

static inline bool is_option_string(const std::string& string)
{
    if(string.find_last_of(OPTION_PREFIX) >= (string.size() - 1)) {
        return false;
    }

    return string[0] == OPTION_PREFIX;
}

static inline std::string get_option(const std::string& string)
{
    std::size_t i;
    for(i = 0; string[i] == OPTION_PREFIX; ++i)
        ;
    return std::string(string.cbegin() + i, string.cend());
}

void cmdline::create(int argc, char** argv)
{
    for(int idx = 1; idx < argc; ++idx) {
        std::string string = argv[idx];

        if(!is_option_string(string)) {
            spdlog::warn("cmdline: non-argument at {}: {}", idx, string);
            continue;
        }

        auto option_string = get_option(string);
        auto next_idx = idx + 1;

        if(next_idx < argc) {
            std::string argument = argv[next_idx];

            if(!is_option_string(argument)) {
                options.insert_or_assign(option_string, argument);
                idx = next_idx;
                continue;
            }
        }

        // The option is either last or has no
        // argument (happens when there is a valid
        // option right next to the one we're parsing)
        options.insert_or_assign(option_string, std::string());
    }
}

void cmdline::insert(const char* option, const char* argument)
{
    if(argument == nullptr) {
        options.insert_or_assign(option, std::string());
    } else {
        options.insert_or_assign(option, argument);
    }
}

const char* cmdline::get(const char* option, const char* fallback)
{
    auto it = options.find(option);

    if(it == options.cend()) {
        return fallback;
    }

    return it->second.c_str();
}

bool cmdline::contains(const char* option)
{
    return options.count(option);
}
