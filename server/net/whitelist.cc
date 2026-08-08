#include "server/pch.hh"

#include "server/net/whitelist.hh"

#include "core/utils/crc64.hh"
#include "core/utils/physfs.hh"

constexpr static std::string_view WHITELIST_FILE = "whitelist.txt";

static emhash8::HashMap<std::uint64_t, ed25519::pkey_type> s_whitelist;

void whitelist::init(void)
{
    std::string line;
    std::istringstream stream;

    if(utils::read_file(WHITELIST_FILE, stream)) {
        while(std::getline(stream, line)) {
            auto pkey = ed25519::import_pkey(line);

            if(pkey.has_value()) {
                auto hash = utils::crc64(pkey.value());

                s_whitelist.insert_or_assign(hash, std::move(pkey.value()));
            }
        }
    }
}

void whitelist::shutdown(void)
{
    std::ostringstream stream;

    for(const auto it : s_whitelist) {
        auto xstr = ed25519::export_pkey(it.second);

        stream << xstr;
        stream << std::endl;
    }

    utils::write_file(WHITELIST_FILE, stream.str());
}

void whitelist::add(const ed25519::pkey_type& pkey)
{
    auto hash = utils::crc64(pkey);

    s_whitelist.insert_or_assign(hash, ed25519::pkey_type(pkey));
}

void whitelist::remove(const ed25519::pkey_type& pkey)
{
    auto hash = utils::crc64(pkey);

    s_whitelist.erase(hash);
}

bool whitelist::contains(const ed25519::pkey_type& pkey)
{
    auto hash = utils::crc64(pkey);

    return s_whitelist.contains(hash);
}
